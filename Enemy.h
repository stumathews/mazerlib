#pragma once
#ifndef ENEMY_H
#define ENEMY_H
#include "Level.h"
#include "ai/BehaviorTree.h"
#include "ai/FSM.h"
#include "ai/FSMTransition.h"
#include "character/Npc.h"
#include "time/PeriodicTimer.h"

class Level;
class Player;
class Room;

class Enemy final : public gamelib::Npc, public std::enable_shared_from_this<Enemy>
{
public:
	Enemy(const std::string& name,
	      const std::string& type, gamelib::Coordinate<int> position,
	      bool visible,
	      const std::shared_ptr<Room>& startRoom, std::shared_ptr<gamelib::AnimatedSprite> sprite,
	      gamelib::Direction startingDirection,
	      std::shared_ptr<const Level> level,
	      std::shared_ptr<gamelib::IGameObjectMoveStrategy> enemyMoveStrategy = nullptr);

	
	void Initialize();

	std::shared_ptr<RoomInfo> CurrentRoom;
	std::shared_ptr<const Level> CurrentLevel;
	std::vector<std::shared_ptr<gamelib::Event>> HandleEvent(const std::shared_ptr<gamelib::Event>& event,
	                                                         const unsigned long deltaMs) override;
	void DoEnemyBehaviors(unsigned long deltaMs);
	bool Move(const unsigned long deltaMs); // true if moved
	void Update(unsigned long deltaMs) override;
	void LoadSettings() override;
	std::string GetSubscriberName() override { return Name; }
	std::string GetName() override { return Name; }

private:
	void CheckForPlayerCollision();
	bool isValidMove{};
	bool IfMoved(gamelib::Direction direction) const;
	bool IsPlayerInSameAxis(const std::shared_ptr<Player>& player, bool verticalView) const;
	void LookForPlayer();
	std::function<void(unsigned long deltaMs)> LookForPlayerAndMove();
	bool IsPlayerInLineOfSight(gamelib::Direction lookDirection) const;
	std::function<void(unsigned long deltaMs)> DoMovingBehavior();
	static bool InSameRoomAsPlayer(std::shared_ptr<Player> player, std::shared_ptr<Room> currentRoom);
	void ConfigureEnemyBehavior();
	std::function<bool()> IfMovedInDirection(gamelib::Direction direction) const; // returns a function returning a bool	
	bool emitMoveEvents{};
	bool moveAtSpeed {};	
	int speed{};
	gamelib::PeriodicTimer moveTimer;
	int moveRateMs{};
	bool animate = true;
	bool drawState = false;
	bool useBehaviorTree = false;

	gamelib::BehaviorTree* behaviorTree = nullptr;
	// NPCs have a state machine which they can configure statically
	gamelib::FSM stateMachine;

	// NPC static states (shared by all NPCs in the Mazer game)
	gamelib::FSMState hitWallState;
	gamelib::FSMState upState;
	gamelib::FSMState downState;
	gamelib::FSMState leftState;
	gamelib::FSMState rightState;

	// NPC state transitions
	gamelib::FSMTransition invalidMoveTransition;
	gamelib::FSMTransition onUpDirection;
	gamelib::FSMTransition onDownDirection;
	gamelib::FSMTransition onLeftDirection;
	gamelib::FSMTransition onRightDirection;
	gamelib::FSMTransition onMovingTransition;
};

#endif
