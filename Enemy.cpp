#include "Enemy.h"
#include <memory>
#include <utility>

#include "RoomInfo.h"
#include "cppgamelib/ai/FSM.h"
#include <cppgamelib/character/MovementAtSpeed.h>
#include "Room.h"
#include <cppgamelib/character/Movement.h>
#include <cppgamelib/events/PlayerMovedEvent.h>
#include <cppgamelib/file/SettingsManager.h>
#include <cppgamelib/geometry/SideUtils.h>
#include "Level.h"
#include "EnemyMovedEvent.h"
#include "EventNumber.h"
#include "GameObjectEventFactory.h"
#include "GameObjectMoveStrategy.h"
#include "cppgamelib/ai/BehaviorTreeBuilder.h"
#include "cppgamelib/ai/InlineBehavioralAction.h"
#include "Player.h"
#include "PlayerCollidedWithEnemyEvent.h"
#include "SDLCollisionDetection.h"
#include "cppgamelib/character/Hotspot.h"
#include "cppgamelib/graphic/DrawableText.h"

namespace mazer
{

	Enemy::Enemy(const std::string& name,
		const std::string& type,
		const gamelib::Coordinate<int> position,
		const bool visible,
		const std::shared_ptr<Room>& startRoom,
		std::shared_ptr<gamelib::AnimatedSprite> sprite,
		const gamelib::Direction startingDirection,
		std::shared_ptr<const Level> level,
		std::shared_ptr<gamelib::IGameObjectMoveStrategy> enemyMoveStrategy)
		: Npc(name, type, position, visible, std::move(sprite), std::move(enemyMoveStrategy)),
		CurrentLevel(std::move(level))
	{
		Npc::SetDirection(startingDirection);

		CurrentRoom = std::make_shared<RoomInfo>(startRoom);
	}

	void Enemy::ConfigureEnemyBehavior()
	{
		// Check which technology to use to control Enemy Behavior

		// ReSharper disable once CppTooWideScope
		const auto useFsm = !useBehaviorTree;

		if (useFsm)
		{
			// Setup Enemy behavior via Finite State Machine
			// Set up possible states enemy can be in
			upState = gamelib::FSMState("Up", LookForPlayerAndMove());
			downState = gamelib::FSMState("Down", LookForPlayerAndMove());
			leftState = gamelib::FSMState("Left", LookForPlayerAndMove());
			rightState = gamelib::FSMState("Right", LookForPlayerAndMove());

			// State when enemy has hit a wall
			hitWallState = gamelib::FSMState("Invalid", [&](const unsigned long deltaMs) { InvertCurrentDirection(); Move(deltaMs); });

			// Set how the states can transition
			invalidMoveTransition = gamelib::FSMTransition([&]()-> bool { return !isValidMove; },
				[&]()-> gamelib::FSMState* { return &hitWallState; });

			// Set up conditions for state transitions
			onUpDirection = gamelib::FSMTransition(IfMovedInDirection(gamelib::Direction::Up),
				[&]()-> gamelib::FSMState* { return &upState; });
			onDownDirection = gamelib::FSMTransition(IfMovedInDirection(gamelib::Direction::Down),
				[&]()-> gamelib::FSMState* { return &downState; });
			onLeftDirection = gamelib::FSMTransition(IfMovedInDirection(gamelib::Direction::Left),
				[&]()-> gamelib::FSMState* { return &leftState; });
			onRightDirection = gamelib::FSMTransition(IfMovedInDirection(gamelib::Direction::Right),
				[&]()-> gamelib::FSMState* { return &rightState; });

			// Configure valid transitions
			upState.Transitions = { onDownDirection, onLeftDirection, onRightDirection, invalidMoveTransition };
			downState.Transitions = { onUpDirection, onLeftDirection, onRightDirection, invalidMoveTransition };
			leftState.Transitions = { onUpDirection, onDownDirection, onRightDirection, invalidMoveTransition };
			rightState.Transitions = { onUpDirection, onDownDirection, onLeftDirection, invalidMoveTransition };
			hitWallState.Transitions = { onUpDirection, onDownDirection, onLeftDirection, onRightDirection };

			// Set state machine to states it can be in
			stateMachine.States = { upState, downState, leftState, rightState, hitWallState };

			// Set the initial state to down
			stateMachine.InitialState = &downState;

			return; // We only use one technology or another 
		}

		if (useBehaviorTree)
		{
			// Setup Enemy Behavior via Behavior Tree

			// Behaviors
			auto* lookForPlayer = new gamelib::InlineBehavioralAction([&](const unsigned long deltaMs)
				{
					LookForPlayer();
					return gamelib::BehaviorResult::Success;
				});

			auto* isInvalidMove = new gamelib::InlineBehavioralAction([&](const unsigned long deltaMs)
				{
					return !isValidMove
						? gamelib::BehaviorResult::Success
						: gamelib::BehaviorResult::Failure;
				});

			auto* invertNpcDirection = new gamelib::InlineBehavioralAction([&](const unsigned long deltaMs)
				{
					InvertCurrentDirection();
					return gamelib::BehaviorResult::Success;
				});

			auto* moveInFacingDirection = new gamelib::InlineBehavioralAction([&](const unsigned long deltaMs)
				{
					if (moveTimer.IsReady())
					{
						// Automatically keep moving our position in configured direction
						Move(deltaMs);

						moveTimer.Reset();
					}

					return gamelib::BehaviorResult::Success;
				});

			// Configure behavior tree
			behaviorTree = BehaviorTreeBuilder()
				.ActiveNodeSelector()
				.Sequence()
				.Action(moveInFacingDirection)
				.Condition(isInvalidMove)
				.Action(invertNpcDirection)
				.Finish()
				.Action(lookForPlayer)
				.Finish()
				.End();

			return; // We only use one technology or another 
		}
	}

	void Enemy::Initialize()
	{
		LoadSettings();
		SubscribeToEvent(gamelib::PlayerMovedEventTypeEventId);
		SubscribeToEvent(SettingsReloadedEventId);

		// Save some frames: don't move every frame. E.g., move every 10ms
		moveTimer.SetFrequency(moveRateMs);

		// Use or create a move strategy. 
		gameObjectMoveStrategy = gameObjectMoveStrategy == nullptr
			? std::make_shared<GameObjectMoveStrategy>(shared_from_this(), CurrentRoom)
			: gameObjectMoveStrategy;

		// Configure enemy behavior after we've initialised the Enemy
		ConfigureEnemyBehavior();
	}

	void Enemy::LoadSettings()
	{
		emitMoveEvents = gamelib::SettingsManager::Bool("enemy", "emitMoveEvents");
		moveAtSpeed = gamelib::SettingsManager::Bool("enemy", "moveAtSpeed");
		speed = gamelib::SettingsManager::Int("enemy", "speed");
		moveRateMs = gamelib::SettingsManager::Int("enemy", "moveRateMs");
		animate = gamelib::SettingsManager::Bool("enemy", "animate");

		// This draw's the enemies state-machine state near/over the enemy itself
		drawState = gamelib::SettingsManager::Bool("enemy", "drawState");

		// Use behavior tree or use Finite state machine
		useBehaviorTree = gamelib::SettingsManager::Bool("enemy", "useBehaviorTree");
	}

	std::vector<std::shared_ptr<gamelib::Event>> Enemy::HandleEvent(const std::shared_ptr<gamelib::Event>& event,
		const unsigned long deltaMs)
	{
		// Only if the player moves...
		if (event->Id == gamelib::PlayerMovedEventTypeEventId)
		{
			CheckForPlayerCollision();
		}

		// Only if settings event occurs
		if (event->Id.PrimaryId == SettingsReloadedEventId.PrimaryId)
		{
			LoadSettings();
		}

		return {};
	}

	void Enemy::Update(const unsigned long deltaMs)
	{
		if (GameData::Get()->IsGameWon())
		{
			return;
		}

		// We only want to move and emit move events periodically. Update the periodic timer
		moveTimer.Update(deltaMs);

		// Do common/normal NPC activities also
		Npc::Update(deltaMs);

		// Do enemy behavior
		DoEnemyBehaviors(deltaMs);
	}

	void Enemy::DoEnemyBehaviors(const unsigned long deltaMs)
	{
		// Select which technology will be used to handle Enemy NPC behavior 
		const auto useFsm = !useBehaviorTree;

		if (useBehaviorTree)
		{
			// Use Behavior Tree for controlling NPC behavior
			if (behaviorTree != nullptr)
			{
				behaviorTree->Update(deltaMs);
			}

			return;
		}

		if (useFsm)
		{
			// Use State Machine for controlling NPC behavior
			stateMachine.Update(deltaMs);

			// Show the current state of the FSM on the enemy
			auto constexpr emptyString = "";
			auto stateText = drawState
				? stateMachine.ActiveState->GetName().substr(0, 1)
				: emptyString;

			// Set the enemy state text
			Status->Text = stateMachine.ActiveState != nullptr
				? stateText
				: emptyString;
		}
	}

	bool Enemy::Move(const unsigned long deltaMs)
	{
		const std::shared_ptr<gamelib::IMovement> movementAtSpeed = std::make_shared<gamelib::MovementAtSpeed>(speed, currentFacingDirection, deltaMs);
		const std::shared_ptr<gamelib::IMovement> constantPixelMovement = std::make_shared<gamelib::Movement>(currentFacingDirection);

		// Move the game object a bit
		isValidMove = gameObjectMoveStrategy->MoveGameObject(moveAtSpeed
			? movementAtSpeed
			: constantPixelMovement);

		if (isValidMove)
		{
			// Tell the world I moved

			if (!emitMoveEvents)
			{
				return true;
			}

			EventSubscriber::RaiseEvent(std::make_shared<EnemyMovedEvent>(shared_from_this()));

			return true;
		}

		return false;
	}

	std::function<bool()> Enemy::IfMovedInDirection(const gamelib::Direction direction) const
	{
		// note: returns a func
		return [this, direction] { return IfMoved(direction); };
	}

	bool Enemy::IfMoved(const gamelib::Direction direction) const
	{
		return isValidMove && currentFacingDirection == direction;
	}

	std::function<void(unsigned long deltaMs)> Enemy::LookForPlayerAndMove()
	{
		return [&](const unsigned long deltaMs)
			{
				LookForPlayer();
				DoMovingBehavior()(deltaMs);
			};
	}

	std::function<void(unsigned long deltaMs)> Enemy::DoMovingBehavior()
	{
		// note: returns a func
		return [this](const unsigned long deltaMs)
			{
				if (moveTimer.IsReady())
				{
					// Automatically keep moving our position in configured direction
					Move(deltaMs);

					moveTimer.Reset();
				}
			};
	}

	void Enemy::LookForPlayer()
	{
		// Most enemy states use this state

		// Get player details
		const auto player = GameData::Get()->GetPlayer();
		const auto playerRow = player->CurrentRoom->GetCurrentRoom()->GetRowNumber(CurrentLevel->NumRows);
		const auto playerCol = player->CurrentRoom->GetCurrentRoom()->GetColumnNumber(CurrentLevel->NumCols);

		// Get enemy (me) details
		const auto currentRoom = CurrentRoom->GetCurrentRoom();
		const auto enemyRow = currentRoom->GetRowNumber(CurrentLevel->NumRows);
		const auto enemyCol = currentRoom->GetColumnNumber(CurrentLevel->NumCols);

		// Don't look player unless player is in same row or column
		if (playerRow != enemyRow && playerCol != enemyCol)
		{
			return;
		}

		// Don't look for player if enemy already in the room, check for collision instead
		if (InSameRoomAsPlayer(player, currentRoom))
		{
			CheckForPlayerCollision();
			return;
		}

		// Should look up and down?
		if (playerCol == enemyCol)
		{
			// Don't look for player if not in the same vertical line as player
			if (IsPlayerInSameAxis(player, false))
			{
				return;
			}

			// Search for player up.
			if (IsPlayerInLineOfSight(gamelib::Direction::Up))
			{
				SetDirection(gamelib::Direction::Up); // chase in direction found
				return;
			}

			// Search for player down.
			if (IsPlayerInLineOfSight(gamelib::Direction::Down))
			{
				SetDirection(gamelib::Direction::Down); //chase in direction found
				return;
			}
		}

		// Should look left and right?
		if (playerRow == enemyRow) // look left and right, chase in direction found
		{
			// Don't look for player if not in the same line as player
			if (IsPlayerInSameAxis(player, true))
			{
				return;
			}

			// Search for player Left.
			if (IsPlayerInLineOfSight(gamelib::Direction::Left))
			{
				SetDirection(gamelib::Direction::Left); //chase in direction found
				return;
			}

			// Search for player right.
			if (IsPlayerInLineOfSight(gamelib::Direction::Right))
			{
				SetDirection(gamelib::Direction::Right); //chase in direction found
			}
		}
	}

	bool Enemy::IsPlayerInSameAxis(const std::shared_ptr<Player>& player, const bool verticalView) const
	{
		const auto playerHotspotPosition = player->Hotspot->GetPosition();
		const auto playerHotspotParentHeight = static_cast<int>(player->Hotspot->ParentHeight);
		const auto playerHotspotParentWidth = static_cast<int>(player->Hotspot->ParentWidth);
		constexpr auto half = 2;

		const auto isWithinVerticalFov =
			TheHotspot->GetPosition().GetY() > playerHotspotPosition.GetY() + playerHotspotParentHeight / half ||
			TheHotspot->GetPosition().GetY() < playerHotspotPosition.GetY() - playerHotspotParentHeight / half;

		const auto isWithinHorizontalFov =
			TheHotspot->GetPosition().GetX() > playerHotspotPosition.GetX() + playerHotspotParentWidth / half ||
			TheHotspot->GetPosition().GetX() < playerHotspotPosition.GetX() - playerHotspotParentWidth / half;

		return verticalView
			? isWithinVerticalFov
			: isWithinHorizontalFov;
	}

	bool Enemy::IsPlayerInLineOfSight(const gamelib::Direction lookDirection) const
	{
		const auto player = GameData::Get()->GetPlayer();

		// Start search in the current room			
		auto currentRoom = CurrentRoom->GetCurrentRoom();
		int nextRoomIndex;

		// Look for line-on-sight in rooms in the direction specified
		// Evaluate each room in that direction to see if its traversable until you reach the end of the map (-1)
		while ((nextRoomIndex = currentRoom->GetNeighborIndex(gamelib::SideUtils::GetSideForDirection(lookDirection))) != -1)
		{
			const auto nextRoom = GameData::Get()->GetRoomByIndex(nextRoomIndex);
			if (lookDirection == gamelib::Direction::Right)
			{
				if (currentRoom->HasRightWall() || nextRoom->HasLeftWall()) return false; // No line of sight
			}

			if (lookDirection == gamelib::Direction::Left)
			{
				if (currentRoom->HasLeftWall() || nextRoom->HasRightWall()) return false; // No line of sight
			}

			if (lookDirection == gamelib::Direction::Up)
			{
				if (currentRoom->HasTopWall() || nextRoom->HasBottomWall()) return false; // No line of sight
			}

			if (lookDirection == gamelib::Direction::Down)
			{
				if (currentRoom->HasBottomWall() || nextRoom->HasTopWall()) return false; // No line of sight
			}

			// pursue next room
			currentRoom = nextRoom;

			// Found the player in this room?
			if (InSameRoomAsPlayer(player, currentRoom))
			{
				// spotted player!
				return true; // Yes!
			}
		}
		return false; // player not found
	}

	// ReSharper disable CppPassValueParameterByConstReference
	bool Enemy::InSameRoomAsPlayer(const std::shared_ptr<Player> player, const std::shared_ptr<Room> currentRoom)  // NOLINT
	{
		return currentRoom->GetRoomNumber() == player->CurrentRoom->RoomIndex;
	}
	// ReSharper restore CppPassValueParameterByConstReference

	void Enemy::CheckForPlayerCollision()
	{
		const auto player = GameDataManager::Get()->TheGameData()->GetPlayer();

		if (InSameRoomAsPlayer(player, CurrentRoom->TheRoom) &&
			SdlCollisionDetection::IsColliding(&player->Bounds, &Bounds))
		{
			RaiseEvent(std::make_shared<PlayerCollidedWithEnemyEvent>(shared_from_this(), player));
			RaiseEvent(GameObjectEventFactory::MakeRemoveObjectEvent(shared_from_this()));
		}
	}
}

