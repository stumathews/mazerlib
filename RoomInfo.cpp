#include "pch.h"
#include "RoomInfo.h"
#include "GameData.h"
#include "Room.h"

RoomInfo::RoomInfo(const std::shared_ptr<Room>& room)
{
	TheRoom = room;
	RoomIndex = room->GetRoomNumber();
}

std::shared_ptr<Room> RoomInfo::GetTopRoom() const
{
	return GetAdjacentRoomTo(GetCurrentRoom(), gamelib::Side::Top);
}

std::shared_ptr<Room> RoomInfo::GetBottomRoom() const
{
	return GetAdjacentRoomTo(GetCurrentRoom(), gamelib::Side::Bottom);
}

std::shared_ptr<Room> RoomInfo::GetRightRoom() const
{
	return GetAdjacentRoomTo(GetCurrentRoom(), gamelib::Side::Right);
}

std::shared_ptr<Room> RoomInfo::GetLeftRoom() const
{
	return GetAdjacentRoomTo(GetCurrentRoom(), gamelib::Side::Left);
}

std::shared_ptr<Room> RoomInfo::GetCurrentRoom() const
{
	return GameData::Get()->GetRoomByIndex(RoomIndex);
}

std::shared_ptr<Room> RoomInfo::GetRoomByIndex(const int index)
{
	return GameData::Get()->GetRoomByIndex(index);
}

std::shared_ptr<Room> RoomInfo::GetAdjacentRoomTo(const std::shared_ptr<Room>& room, const gamelib::Side side)
{
	if (!room) { return nullptr; }
	return GameData::Get()->GetRoomByIndex(room->GetNeighborIndex(side));
}

void RoomInfo::SetCurrentRoom(const std::shared_ptr<Room>& room)
{
	RoomIndex = room->GetRoomNumber();
	TheRoom = room;
}
