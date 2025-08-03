#pragma once
#include <vector>
#include <memory>
#include "Room.h"

class RoomGenerator
{
public:
	RoomGenerator() = delete;
	RoomGenerator(int screenWidth, int screenHeight, int rows, int columns, bool removeRandomSides);
	
	void ConfigureRooms(const std::vector<std::shared_ptr<Room>>& rooms) const;
	void ConfigureWalls(const std::shared_ptr<Room>& thisRoom, const bool& canRemoveWallAbove,
	                    const std::vector<std::shared_ptr<Room>>& rooms, const std::shared_ptr<Room>& nextRoom,
	                    const bool& canRemoveWallRight, const bool& canRemoveWallBelow, const bool& canRemoveWallLeft,
	                    const int& prevIndex) const;
	void RemoveSidesRandomly(const bool& canRemoveAbove, const std::shared_ptr<Room>& currentRoom,
	                         const std::vector<std::shared_ptr<Room>>& rooms, const std::shared_ptr<Room>& nextRoom,
	                         const bool& canRemoveRight, const bool& canRemoveBelow, const bool& canRemoveLeft,
	                         const int& prevIndex) const;

	[[nodiscard]] std::vector<std::shared_ptr<Room>> Generate() const;

private:
	int screenWidth, screenHeight, rows, columns;
	bool removeRandomSides;
};
