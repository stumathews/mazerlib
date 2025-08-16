#include "Player.h"
#include <memory>
#include <cppgamelib/events/EventFactory.h>
#include <cppgamelib/events/EventManager.h>
#include <cppgamelib/file/SettingsManager.h>

#include "EventNumber.h"
//#include "GameData.h"
//#include "Room.h"
#include "RoomInfo.h"
#include <cppgamelib/events/PlayerMovedEvent.h>
#include <character/AnimatedSprite.h>
#include <character/Hotspot.h>
//#include "GameObjectMoveStrategy.h"
//#include "GameObjectMoveStrategy.h"
#include "GameData.h"
#include "character/IGameMoveStrategy.h"
//#include "cppgamelib/character/StatefulMove.h"

using namespace std;
using namespace gamelib;

namespace mazer
{
	Player::Player(const std::string& name,
		const std::string& type,
		const Coordinate<int> position, // we know the position the player is at
		const int width,
		const int height,
		const std::string& identifier)
		: DrawableGameObject(name, type, position, true), CurrentMovingDirection(Direction::Down),
		CurrentFacingDirection(Direction::Down)
	{
		CommonInit(width, height, identifier);
	}

	Player::Player(const std::string& name, const std::string& type,
		const std::shared_ptr<Room>& playerRoom, // we know the room the player will be in
		const int playerWidth,
		const int playerHeight,
		const std::string& identifier)
		: DrawableGameObject(name, type, playerRoom->GetCenter(playerWidth, playerHeight), true),
		CurrentMovingDirection(Direction::Down), CurrentFacingDirection(Direction::Down)
	{
		CommonInit(playerWidth, playerHeight, identifier);
		CurrentRoom = make_shared<RoomInfo>(playerRoom);
		CenterPlayerInRoom(playerRoom);
	}

	Player::Player(const std::string& name, const std::string& type, const std::shared_ptr<Room>& playerRoom,
		const AbcdRectangle& dimensions, const std::string& identifier)
		: DrawableGameObject(name, type, playerRoom->GetCenter(dimensions.GetWidth(), dimensions.GetHeight()), true),
		CurrentMovingDirection(Direction::Down), CurrentFacingDirection(Direction::Down)
	{
		CommonInit(dimensions.GetWidth(), dimensions.GetWidth(), identifier);
		CurrentRoom = make_shared<RoomInfo>(playerRoom);
		CenterPlayerInRoom(playerRoom);
	}

	Player::Player(const std::string& name,
		const std::string& type,
		const std::shared_ptr<Room>& playerRoom,
		const std::string& identifier)
		: DrawableGameObject(name, type, playerRoom->GetCenter(0, 0), true),
		CurrentMovingDirection(Direction::Down), CurrentFacingDirection(Direction::Down)
	{
		CommonInit(0, 0, identifier); // Height / Width set by setting the asset
		CurrentRoom = make_shared<RoomInfo>(playerRoom);
		CenterPlayerInRoom(playerRoom);
	}

	void Player::CommonInit(const int playerWidth, const int playerHeight, const std::string& identifier)
	{
		Width = playerWidth;
		Height = playerHeight;
		Sprite = nullptr;
		CurrentMovingDirection = Direction::Up;
		CurrentFacingDirection = this->CurrentMovingDirection;
		Identifier = identifier;

		DirectionKeyStates =
		{
			{ Direction::Up , ControllerMoveEvent::KeyState::Unknown },
			{ Direction::Down , ControllerMoveEvent::KeyState::Unknown },
			{ Direction::Left , ControllerMoveEvent::KeyState::Unknown },
			{ Direction::Right , ControllerMoveEvent::KeyState::Unknown },
		};

		UpdateBounds(Width, Height);

		SubscribeToEvent(ControllerMoveEventId); // player wants to know when the controller moves
		SubscribeToEvent(SettingsReloadedEventId); // wants to know when settings are reloaded
		SubscribeToEvent(FireEventId); // wants to know when the fire button is pressed
		SubscribeToEvent(GameWonEventId); // want to know when the game is won
	}

	void Player::LoadSettings()
	{
		GameObject::LoadSettings();

		drawBounds = SettingsManager::Get()->GetBool("player", "drawBounds");
		verbose = SettingsManager::Get()->GetBool("global", "verbose");
		pixelsToMove = SettingsManager::Get()->GetInt("player", "pixelsToMove");
		hotspotSize = SettingsManager::Get()->GetInt("player", "hotspotSize");
		drawHotSpot = SettingsManager::Get()->GetBool("player", "drawHotspot");
		hideSprite = SettingsManager::Get()->GetBool("player", "hideSprite");
		speed = SettingsManager::Get()->Int("player", "speed");
		moveRateMs = SettingsManager::Int("enemy", "moveRateMs");

		moveTimer.SetFrequency(moveRateMs);
	}

	void Player::LogMessage(const char* str, bool verbose)
	{
	}

	ListOfEvents Player::HandleEvent(const std::shared_ptr<Event>& event, const unsigned long deltaMs)
	{
		ListOfEvents createdEvents{};

		if (event->Id.PrimaryId == ControllerMoveEventId.PrimaryId)
		{
			return OnControllerMove(event, createdEvents, deltaMs);
		}

		if (event->Id.PrimaryId == FireEventId.PrimaryId)
		{
			Fire();
		}

		if (event->Id.PrimaryId == SettingsReloadedEventId.PrimaryId)
		{
			LoadSettings();
		}

		if (event->Id.PrimaryId == InvalidMoveEventId.PrimaryId)
		{
			LogMessage("Invalid move", verbose);
		}

		if (event->Id.PrimaryId == GameWonEventId.PrimaryId)
		{
			OnGameWon();
		}

		return createdEvents;
	}

	const ListOfEvents& Player::OnControllerMove(const shared_ptr<Event>& event, ListOfEvents& createdEvents, const unsigned long deltaMs)
	{
		if (gameWon)
		{
			return createdEvents;
		}

		const auto moveEvent = dynamic_pointer_cast<ControllerMoveEvent>(event);
		const auto moveDirection = moveEvent->Direction;

		// Set acceleration in direction depending on if direction key is pressed or not
		DirectionKeyStates[moveDirection] = moveEvent->GetKeyState();

		SetPlayerDirection(moveDirection);

		return createdEvents;
	}

	void Player::Update(const unsigned long deltaMs)
	{
		if (GameData::Get()->IsGameWon()) return;

		moveTimer.Update(deltaMs);

		// We don't move every single frame...
		moveTimer.DoIfReady([&]()
			{
				const auto keyStateBackup = DirectionKeyStates;

				CancelInvalidDirectionKeyPresses(DirectionKeyStates);

				Move(deltaMs);

				// restore original key states (e.g. the temporarily cancelled keypress my now be acceptable)
				DirectionKeyStates = keyStateBackup;
			});
	}

	void Player::CancelInvalidDirectionKeyPresses(std::map<Direction, ControllerMoveEvent::KeyState>& currentKeyStates)
	{
		// If there are any directions that are invalid, temporarily cancel the direction's keypress
		for (const auto& [direction, keyState] : currentKeyStates)
		{
			if (keyState == ControllerMoveEvent::KeyState::Pressed && !moveStrategy->CanGameObjectMove(direction))
			{
				DirectionKeyStates[direction] = ControllerMoveEvent::KeyState::Released;
			}
		}
	}

	void Player::Move(const unsigned long deltaMs)
	{
		//const auto movement = std::make_shared<StatefulMove>(speed, DirectionKeyStates, deltaMs);

		// Move player
		//const auto isValidMove = moveStrategy->MoveGameObject(movement);	

		auto isValidMove = true;
		if (!isValidMove)
		{
			EventManager::Get()->RaiseEvent(EventFactory::Get()->CreateGenericEvent(InvalidMoveEventId, GetName()), this);
		}

		// Move player's sprite also
		if (Sprite)
		{
			// Only animate sprite if there is no direction set
			/*if(movement->GetDirection() != Direction::None)
			{
				Sprite->Update(deltaMs, AnimatedSprite::GetStdDirectionAnimationFrameGroup(movement->GetDirection()));
			}*/

			Sprite->MoveSprite(Position.GetX(), Position.GetY());
		}

		Hotspot->Update(Position);

		UpdateBounds(Width, Height);

		// Only register a move if there was a move in a known direction
		/*if(movement->GetDirection() != Direction::None)
		{
			EventManager::Get()->RaiseEvent(EventFactory::Get()->CreatePlayerMovedEvent(movement->GetDirection()), this);
		}*/
	}

	void Player::Draw(SDL_Renderer* renderer)
	{
		// Draw
		if (!hideSprite) { Sprite->Draw(renderer); }
		if (drawHotSpot) { Hotspot->Draw(renderer); }

		// Debugging
		if (drawBounds)
		{
			constexpr SDL_Color colour = { 255, 0, 0, 0 };
			SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
			SDL_RenderDrawRect(renderer, &Bounds);
		}
	}

	void Player::SetPlayerDirection(const Direction direction)
	{
		CurrentMovingDirection = direction;
		CurrentFacingDirection = direction;
	}

	void Player::Fire() const
	{
		RemovePlayerFacingWall();
	}

	int Player::GetPoints()
	{
		return IntProperties["Points"];
	}

	std::string Player::GetName()
	{
		return Name;
	}

	void Player::OnGameWon()
	{
		gameWon = true;
	}

	int Player::GetHealth()
	{
		return IntProperties["Health"];
	}

	void Player::SetSprite(const std::shared_ptr<AnimatedSprite>& inSprite)
	{
		Sprite = inSprite;
		Width = inSprite->Dimensions.GetWidth();
		Height = inSprite->Dimensions.GetHeight();

		Hotspot = std::make_shared<gamelib::Hotspot>(Position, Width, Height, hotspotSize);

		CalculateBounds(Position, Width, Height);
	}

	void Player::SetMoveStrategy(const std::shared_ptr<IGameObjectMoveStrategy>& inMoveStrategy)
	{
		moveStrategy = inMoveStrategy;
	}

	inline int Player::GetHotSpotLength() const
	{
		return hotspotSize;
	}

	int Player::GetWidth() const
	{
		return Width;
	}

	int Player::GetHeight() const
	{
		return Height;
	}

	void Player::RemovePlayerFacingWall() const
	{
		switch (CurrentFacingDirection)
		{
		case Direction::Up: RemoveTopWall(); break;
		case Direction::Down: RemoveBottomWall(); break;
		case Direction::Left: RemoveLeftWall(); break;
		case Direction::Right: RemoveRightWall(); break;
		case Direction::None: break;
		}
	}

	void Player::RemoveRightWall() const
	{
		if (const auto rightRoom = CurrentRoom->GetRightRoom())
		{
			CurrentRoom->TheRoom->RemoveWall(Side::Right);
			rightRoom->RemoveWall(Side::Left);
		}
	}

	void Player::RemoveLeftWall() const
	{
		if (const auto leftRoom = CurrentRoom->GetLeftRoom())
		{
			CurrentRoom->TheRoom->RemoveWall(Side::Left);
			leftRoom->RemoveWall(Side::Right);
		}
	}

	void Player::RemoveBottomWall() const
	{
		if (const auto bottomRoom = CurrentRoom->GetBottomRoom())
		{
			CurrentRoom->TheRoom->RemoveWall(Side::Bottom);
			bottomRoom->RemoveWall(Side::Top);
		}
	}

	void Player::RemoveTopWall() const
	{
		if (const auto topRoom = CurrentRoom->GetTopRoom())
		{
			CurrentRoom->TheRoom->RemoveWall(Side::Top);
			topRoom->RemoveWall(Side::Bottom);
		}
	}

	void Player::CenterPlayerInRoom(const shared_ptr<Room>& targetRoom)
	{
		// local func
		const function<Coordinate<int>(Room, Player)> centerPlayerFunc = [](const Room& room, const Player& p)
			{
				const auto roomXMid = room.GetX() + (room.GetWidth() / 2);
				const auto roomYMid = room.GetY() + (room.GetHeight() / 2);
				const auto x = roomXMid - p.Width / 2;
				const auto y = roomYMid - p.Height / 2;
				return Coordinate(x, y);
			};

		const auto coords = centerPlayerFunc(*targetRoom, *this);
		Position.SetY(coords.GetY());
		Position.SetX(coords.GetX());
	}
}
