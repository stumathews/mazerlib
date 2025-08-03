#pragma once
#include <memory>

#include "Room.h"

class RoomInfo
{
public:
	explicit RoomInfo(const std::shared_ptr<Room>& room);

	[[nodiscard]] std::shared_ptr<Room> GetTopRoom() const;
	[[nodiscard]] std::shared_ptr<Room> GetBottomRoom() const;
	[[nodiscard]] std::shared_ptr<Room> GetRightRoom() const;
	[[nodiscard]] std::shared_ptr<Room> GetLeftRoom() const;
	[[nodiscard]] std::shared_ptr<Room> GetCurrentRoom() const;

	static std::shared_ptr<Room> GetRoomByIndex(int index);
	static std::shared_ptr<Room> GetAdjacentRoomTo(const std::shared_ptr<Room>& room, gamelib::Side side);
	void SetCurrentRoom(const std::shared_ptr<Room>& room);
		
	std::shared_ptr<Room> TheRoom;
	int RoomIndex = 0;

};
