#pragma once
#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <memory>
#include <map>
#include <vector>
#include <objects/GameWorldData.h>
#include "Enemy.h"

namespace gamelib
{
	class GameObject;
	class Pickup;
}

class Room;
class Player;


/**
 * \brief Holds Game Data that is updated periodically by the Game Data Manager
 */
class GameData : public gamelib::GameWorldData
{
public:
	static GameData* Get();
	GameData(const GameData&) = delete;
	GameData(const GameData&&) = delete;
	GameData& operator=(const GameData&) = delete;
	GameData& operator=(const GameData&&) = delete;
	~GameData() { instance = nullptr; }

	void AddRoom(const std::shared_ptr<Room>& room);
	void RemoveRoom(const std::shared_ptr<Room>& room);
	void AddPickup(std::shared_ptr<gamelib::Pickup> pickup);
	void RemovePickup(const std::shared_ptr<gamelib::Pickup>& pickup);
	void AddEnemy(std::shared_ptr<Enemy> enemy);
	void RemoveEnemy(const std::shared_ptr<Enemy>& enemy);

	std::shared_ptr<Room> GetRoomByIndex(int roomNumber);
	[[nodiscard]]
	std::shared_ptr<Player> GetPlayer() const;
	[[nodiscard]]
	unsigned int CountPickups() const { return static_cast<unsigned int>(pickups.size()); }
	[[nodiscard]]
	bool IsGameWon() const { return isGameWon; }
	void SetGameWon(const bool yesNo) { isGameWon = yesNo; }
	void AddGameObject(const std::shared_ptr<gamelib::GameObject>& gameObject);
	void RemoveGameObject(const std::shared_ptr<gamelib::GameObject>& gameObject);
	void RemoveExpiredReferences();
	void Clear();

	std::vector<std::weak_ptr<gamelib::GameObject>> GameObjects;
	std::vector<std::weak_ptr<Enemy>> Enemies() { return enemies; }

protected:
	GameData();
	static GameData* instance;

private:
	static bool IsSameId(const std::weak_ptr<gamelib::GameObject>& obj,
	                     const std::shared_ptr<gamelib::GameObject>& other);
	bool isGameWon;
	std::map<int, std::weak_ptr<Room>> rooms;
	std::vector<std::weak_ptr<gamelib::Pickup>> pickups;
	std::vector<std::weak_ptr<Enemy>> enemies;
};

#endif
