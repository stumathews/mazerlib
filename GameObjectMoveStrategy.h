#pragma once

//#include <cppgamelib/character/Direction.h>

#include <cppgamelib/character/Direction.h>

#include "ElapsedGameTimeProvider.h"
#include "character/IGameMoveStrategy.h"


namespace mazer
{
	class RoomInfo;
	class Player;
	class Room;

	// The GameObjectMoveStrategy is very useful. It not only can it move a game object but will also predict if that move is valid or not.
	// It does this by assessing a potential move's validity relative the current room.
	//
	// So the GameObjectMoveStrategy is very short-sighted, it only knows/sees what room it is and assesses the potential to move relative to that room.
	// This means it can be used by a NPC to move itself (MoveGameObject) or determine if the potential move is invalid (IsValidMove)
	//
	// For example, by looking at its current room, it knows it cannot move left if it has a wall on the left.
	// This means the object is the eyes and brains of the NPC. It contains the rules and observes the restrictions of movement relative to the current room.
	//
	// You give it your game object and the current room, and you can ask it to move the game object, and it will manipulate the gme object's position
	// or tell you that the move is not possible.
	class GameObjectMoveStrategy final : public gamelib::IGameObjectMoveStrategy
	{
	public:
		GameObjectMoveStrategy(const std::shared_ptr<gamelib::GameObject>& gameObject, const std::shared_ptr<RoomInfo>& currentRoom);

		// Move the game object (movement encodes the desired direction and pixels to move)
		bool MoveGameObject(std::shared_ptr<gamelib::IMovement> movement) override;

		// Determine if the game object can move in the intended direction
		[[nodiscard]] bool CanGameObjectMove(gamelib::Direction direction) override;

	private:

		void SetGameObjectPosition(gamelib::Coordinate<float> resultingMove) const;
		[[nodiscard]] gamelib::Coordinate<float> CalculateGameObjectMove(const std::shared_ptr<gamelib::IMovement>& movement,
			int pixelsToMove) const;
		[[nodiscard]] bool IsValidMove(const std::shared_ptr<gamelib::IMovement>& movement) override;

		std::shared_ptr<gamelib::GameObject> gameObject;
		std::shared_ptr<RoomInfo> roomInfo;
		bool ignoreRestrictions;
		bool debug;
	};
}
