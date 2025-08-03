#pragma once
#include <geometry/Coordinate.h>
#include <character/IMovement.h>
#include "RoomInfo.h"
#include "character/IGameMoveStrategy.h"

class Player;
class Room;

// Moves a Game Object in some way
class GameObjectMoveStrategy final : public gamelib::IGameObjectMoveStrategy
{
public:
	GameObjectMoveStrategy(const std::shared_ptr<gamelib::GameObject>& gameObject,
	                       const std::shared_ptr<RoomInfo>& roomInfo);

	// Move it
	bool MoveGameObject(std::shared_ptr<gamelib::IMovement> movement) override;
	[[nodiscard]] bool CanGameObjectMove(gamelib::Direction direction) override;

private:


	void SetGameObjectPosition(gamelib::Coordinate<int> resultingMove) const;	
	[[nodiscard]] gamelib::Coordinate<int> CalculateGameObjectMove(const std::shared_ptr<gamelib::IMovement>& movement,
	                                                               int pixelsToMove) const;
	[[nodiscard]] bool IsValidMove(const std::shared_ptr<gamelib::IMovement>& movement) override;

	std::shared_ptr<gamelib::GameObject> gameObject;
	std::shared_ptr<RoomInfo> roomInfo;
	bool ignoreRestrictions;
	bool debug;
};
