#include "pch.h"
#include "GameObjectMoveStrategy.h"

#include "Player.h"
#include "Room.h"
#include <character/Direction.h>
#include <file/Logger.h>
#include <exceptions/EngineException.h>
#include <file/SettingsManager.h>
#include "../LevelManager.h"
#include "character/IMovement.h"

GameObjectMoveStrategy::GameObjectMoveStrategy(const std::shared_ptr<gamelib::GameObject>& gameObject,
                                               const std::shared_ptr<RoomInfo>& roomInfo)
{
	this->gameObject = gameObject;
	this->roomInfo = roomInfo;

	// Read some config for moving
	debug = gamelib::SettingsManager::Bool("player", "debugMovement");
	ignoreRestrictions = gamelib::SettingsManager::Bool("player", "ignoreRestrictions");
}

bool GameObjectMoveStrategy::MoveGameObject(const std::shared_ptr<gamelib::IMovement> movement)
{
	auto isMoveValid = false;
	if (IsValidMove(movement))
	{
		// Calculate move
		const auto newPosition = movement->SupportsPositionalMovement() 
			                         ? movement->GetPosition(gameObject->Position)
			                         : CalculateGameObjectMove(movement, movement->GetPixelsToMove());

		// Move
		SetGameObjectPosition(newPosition);
		isMoveValid = true;
	}
	return isMoveValid;
}

gamelib::Coordinate<int> GameObjectMoveStrategy::CalculateGameObjectMove(
	const std::shared_ptr<gamelib::IMovement>& movement, const int pixelsToMove) const
{
	int y = gameObject->Position.GetY();
	int x = gameObject->Position.GetX();

	switch (movement->GetDirection())
	{
		case gamelib::Direction::Down: y += pixelsToMove;	break;
		case gamelib::Direction::Up: y -= pixelsToMove; 	break;
		case gamelib::Direction::Left: x -= pixelsToMove;  break;
		case gamelib::Direction::Right: x += pixelsToMove; break;
		case gamelib::Direction::None: THROW(0, "Direction is NOne", "PlayerMoveStrategy");
	}

	return {x, y};
}

void GameObjectMoveStrategy::SetGameObjectPosition(const gamelib::Coordinate<int> resultingMove) const
{
	// Actually move the underlying game object by modifying it
	gameObject->Position.SetX(resultingMove.GetX());
	gameObject->Position.SetY(resultingMove.GetY());
}

bool GameObjectMoveStrategy::IsValidMove(const std::shared_ptr<gamelib::IMovement>& movement)
{
	if (ignoreRestrictions) { return true; }
	
	switch (movement->GetDirection())
	{
		case gamelib::Direction::Down: return CanGameObjectMove(gamelib::Direction::Down);
		case gamelib::Direction::Left: return CanGameObjectMove(gamelib::Direction::Left);
		case gamelib::Direction::Right: return CanGameObjectMove(gamelib::Direction::Right);
		case gamelib::Direction::Up: return CanGameObjectMove(gamelib::Direction::Up);
		case gamelib::Direction::None: return true; // moving in no direction is a valid move
	}
	return false;
}

bool GameObjectMoveStrategy::CanGameObjectMove(const gamelib::Direction direction)
{
	std::shared_ptr<Room> targetRoom;
	bool touchingBlockingWalls = false;
	bool hasValidTargetRoom; // is the determined target room valid?
	const auto currentRoom = roomInfo->GetCurrentRoom();

	if (!currentRoom) { return false; }

	auto intersectsRectAndLine = [=](const SDL_Rect bounds, gamelib::Line line) -> bool
	{
		return SDL_IntersectRectAndLine(&bounds, &line.X1, &line.Y1, &line.X2, &line.Y2);
	};

	if (direction == gamelib::Direction::Right)
	{
		targetRoom = roomInfo->GetRightRoom();
		hasValidTargetRoom = targetRoom != nullptr;
		touchingBlockingWalls =
			(hasValidTargetRoom && targetRoom->HasLeftWall() && intersectsRectAndLine(
				gameObject->Bounds, targetRoom->LeftLine)) ||
			currentRoom->HasRightWall() && intersectsRectAndLine(gameObject->Bounds, currentRoom->RightLine);
	}
	else if (direction == gamelib::Direction::Left)
	{
		targetRoom = roomInfo->GetLeftRoom();
		hasValidTargetRoom = targetRoom != nullptr;

		touchingBlockingWalls =
			(hasValidTargetRoom && targetRoom->HasRightWall() && intersectsRectAndLine(
				gameObject->Bounds, targetRoom->RightLine)) ||
			currentRoom->HasLeftWall() && intersectsRectAndLine(gameObject->Bounds, currentRoom->LeftLine);
	}
	else if (direction == gamelib::Direction::Up)
	{
		targetRoom = roomInfo->GetTopRoom();
		hasValidTargetRoom = targetRoom != nullptr;
		touchingBlockingWalls =
			(hasValidTargetRoom && targetRoom->HasBottomWall() && intersectsRectAndLine(
				gameObject->Bounds, targetRoom->BottomLine)) ||
			currentRoom->HasTopWall() && intersectsRectAndLine(gameObject->Bounds, currentRoom->TopLine);
	}
	else if (direction == gamelib::Direction::Down)
	{
		targetRoom = roomInfo->GetBottomRoom();
		hasValidTargetRoom = targetRoom != nullptr;
		touchingBlockingWalls =
			(hasValidTargetRoom && targetRoom->HasTopWall() && intersectsRectAndLine(
				gameObject->Bounds, targetRoom->TopLine)) ||
			currentRoom->HasBottomWall() && intersectsRectAndLine(gameObject->Bounds, currentRoom->BottomLine);
	}

	return !touchingBlockingWalls;
}
