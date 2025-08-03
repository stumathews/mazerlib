#include "pch.h"
#include "Rooms.h"
#include <file/SettingsManager.h>
#include <functional>

using namespace gamelib;

void Rooms::ConfigureRooms(const int rows, const int columns, const std::vector<std::shared_ptr<Room>>& rooms)
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
		const auto canRemoveTopWall = roomRow >= 1 && roomRow <= (rows - 1);
		const auto canRemoveBottomWall = roomRow >= 0 && roomRow < (rows - 1);
		const auto canRemoveLeftWall = roomCol > 0 && roomCol <= (columns - 1);
		const auto canRemoveRightWall = roomCol >= 0 && roomCol < (columns - 1);

		// Calculate indexes of sorrounding/adjacent rooms
		const auto roomIndexAbove = canRemoveTopWall ? (i - columns) : -1;
		const auto roomIndexBelow = canRemoveBottomWall ? (i + columns) : -1;
		const auto roomIndexLeft = canRemoveLeftWall ? prevIndex : -1;
		const auto roomIndexRight = canRemoveRightWall ? nextIndex : -1;
		const auto& thisRoom = rooms[i];
		auto nextRoom = nextIndex == totalRooms ? nullptr : rooms[nextIndex];

		thisRoom->SetSurroundingRooms(roomIndexAbove, roomIndexRight, roomIndexBelow, roomIndexLeft, rooms);

		ConfigureWalls(thisRoom);
	}
}

Coordinate<int> Rooms::CenterOfRoom(const std::shared_ptr<Room>& room, const int yourWidth, const int yourHeight)
{
	// local func to the center the player in the given room
	const std::function<Coordinate<int>(Room, int, int)> centerPlayerFunc = [
		](const Room& inRoom, const int w, const int h)
	{
		const auto roomXMid = inRoom.GetX() + (inRoom.GetWidth() / 2);
		const auto roomYMid = inRoom.GetY() + (inRoom.GetHeight() / 2);
		const auto x = roomXMid - w / 2;
		const auto y = roomYMid - h / 2;
		return Coordinate(x, y);
	};

	return centerPlayerFunc(*room, yourWidth, yourHeight);
}

void Rooms::ConfigureWalls(const std::shared_ptr<Room>& thisRoom)
{
	if (SettingsManager::Get()->GetBool("grid", "nowalls"))
	{
		RemoveAllWalls(thisRoom);
	}
}

void Rooms::RemoveAllWalls(const std::shared_ptr<Room>& thisRoom)
{
	thisRoom->RemoveWall(Side::Top);
	thisRoom->RemoveWall(Side::Right);
	thisRoom->RemoveWall(Side::Bottom);
	thisRoom->RemoveWall(Side::Left);
}
