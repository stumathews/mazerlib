#include "pch.h"
#include "RoomGenerator.h"
#include <algorithm>
#include <random>
#include <vector>
#include <file/SettingsManager.h>
#include "Rooms.h"

using namespace std;
using namespace gamelib;

RoomGenerator::RoomGenerator(const int screenWidth, const int screenHeight, const int rows, const int columns,
                             const bool removeRandomSides)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	this->rows = rows;
	this->columns = columns;
	this->removeRandomSides = removeRandomSides;
}

vector<shared_ptr<Room>> RoomGenerator::Generate() const
{
	vector<shared_ptr<Room>> rooms;
	auto count = 0;
	const auto squareWidth = screenWidth / columns;
	const auto squareHeight = screenHeight / rows;

	for (auto row = 0; row < rows; row++)
	{
		for (auto col = 0; col < columns; col++)
		{
			const auto number = count++;
			auto roomName = string("Room") + std::to_string(number);
			auto room = std::make_shared<Room>(roomName, "Room", number, col * squareWidth, row * squareHeight,
			                                   squareWidth, squareHeight, false);
			room->SetTag(std::to_string(number));
			rooms.push_back(room);
		}
	}

	ConfigureRooms(rooms);

	return rooms;
}

void RoomGenerator::ConfigureRooms(const std::vector<std::shared_ptr<Room>>& rooms) const
{
	const auto totalRooms = static_cast<int>(rooms.size());

	// Setup room
	for (auto i = 0; i < totalRooms; i++)
	{
		const auto nextIndex = i + 1;
		const auto prevIndex = i - 1;
		const auto roomRow = static_cast<int>(ceil(i / columns));
		const auto roomCol = i % columns;

		// Prevent removing outer walls 
		auto canRemoveTopWall = roomRow >= 1 && roomRow <= rows - 1;
		auto canRemoveBottomWall = roomRow >= 0 && roomRow < rows - 1;
		auto canRemoveLeftWall = roomCol > 0 && roomCol <= columns - 1;
		auto canRemoveRightWall = roomCol >= 0 && roomCol < columns - 1;

		// Calculate indexes of sorrounding/adjacent rooms
		const auto roomIndexAbove = canRemoveTopWall ? i - columns : i;
		const auto roomIndexBelow = canRemoveBottomWall ? i + columns : i;
		const auto roomIndexLeft = canRemoveLeftWall ? prevIndex : i;
		const auto roomIndexRight = canRemoveRightWall ? nextIndex : i;
		auto& thisRoom = rooms[i];
		auto& nextRoom = nextIndex == totalRooms ? rooms[i] : rooms[nextIndex];

		thisRoom->SetSurroundingRooms(roomIndexAbove, roomIndexRight, roomIndexBelow, roomIndexLeft, rooms);

		ConfigureWalls(thisRoom, canRemoveTopWall, rooms, nextRoom, canRemoveRightWall, canRemoveBottomWall,
		               canRemoveLeftWall, prevIndex);
	}
}

void RoomGenerator::ConfigureWalls(const std::shared_ptr<Room>& thisRoom, const bool& canRemoveWallAbove,
                                   const std::vector<std::shared_ptr<Room>>& rooms,
                                   const std::shared_ptr<Room>& nextRoom, const bool& canRemoveWallRight,
                                   const bool& canRemoveWallBelow, const bool& canRemoveWallLeft,
                                   const int& prevIndex) const
{
	if (SettingsManager::Get()->GetBool("grid", "nowalls"))
	{
		Rooms::RemoveAllWalls(thisRoom);
		return;
	}

	RemoveSidesRandomly(canRemoveWallAbove, thisRoom, rooms, nextRoom, canRemoveWallRight, canRemoveWallBelow,
	                    canRemoveWallLeft, prevIndex);
}


void RoomGenerator::RemoveSidesRandomly(const bool& canRemoveAbove, const std::shared_ptr<Room>& currentRoom,
                                        const std::vector<std::shared_ptr<Room>>& rooms,
                                        const std::shared_ptr<Room>& nextRoom,
                                        const bool& canRemoveRight, const bool& canRemoveBelow,
                                        const bool& canRemoveLeft, const int& prevIndex) const
{
	vector<Side> removableSides;

	if (canRemoveAbove) { removableSides.push_back(Side::Top); }
	if (canRemoveBelow) { removableSides.push_back(Side::Bottom); }
	if (canRemoveLeft) { removableSides.push_back(Side::Left); }
	if (canRemoveRight) { removableSides.push_back(Side::Right); }

	if (removeRandomSides)
	{
		vector<Side> sidesToSample;
		std::sample(begin(removableSides), end(removableSides), std::back_inserter(sidesToSample), 1,
		            std::mt19937{std::random_device{}()});

		const auto randomSide = sidesToSample.front();

		if (randomSide == Side::Top && canRemoveAbove)
		{
			currentRoom->RemoveWallZeroBased(Side::Top);
			const auto& roomAbove = rooms[currentRoom->GetNeighborIndex(Side::Top)];

			roomAbove->RemoveWallZeroBased(Side::Bottom);
			nextRoom->RemoveWallZeroBased(Side::Bottom);
		}

		if (randomSide == Side::Right && canRemoveRight)
		{
			currentRoom->RemoveWallZeroBased(Side::Right);
			const auto& roomToLeft = rooms[currentRoom->GetNeighborIndex(Side::Right)];

			roomToLeft->RemoveWallZeroBased(Side::Left);
			nextRoom->RemoveWallZeroBased(Side::Left);
		}

		if (randomSide == Side::Bottom && canRemoveBelow)
		{
			currentRoom->RemoveWallZeroBased(Side::Bottom);
			const auto& roomBelow = rooms[currentRoom->GetNeighborIndex(Side::Bottom)];

			roomBelow->RemoveWallZeroBased(Side::Top);
			nextRoom->RemoveWallZeroBased(Side::Top);
		}

		if (randomSide == Side::Left && canRemoveLeft)
		{
			currentRoom->RemoveWallZeroBased(Side::Left);
			const auto& prev = rooms[prevIndex];

			const auto& roomOnLeft = rooms[currentRoom->GetNeighborIndex(Side::Left)];

			roomOnLeft->RemoveWallZeroBased(Side::Right);
			prev->RemoveWallZeroBased(Side::Right);
		}
	}
}
