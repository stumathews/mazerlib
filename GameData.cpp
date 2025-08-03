#include "GameData.h"
#include "room.h"
#include <memory>
#include <objects/GameObject.h>
#include <vector>
#include "pickup.h"
#include "Player.h"

using namespace gamelib;
using namespace std;

GameData::GameData(): isGameWon(false) { }

void GameData::Clear()
{
	enemies.clear();
	pickups.clear();
	rooms.clear();
	GameObjects.clear();
	isGameWon = false;
	IsNetworkGame = false;
}

GameData* GameData::instance = nullptr;

void GameData::AddRoom(const std::shared_ptr<Room>& room)
{
	rooms[room->GetRoomNumber()] = room;
	AddGameObject(room);
}

void GameData::RemoveRoom(const std::shared_ptr<Room>& room)
{
	rooms.erase(room->GetRoomNumber());
	RemoveGameObject(room);
}

void GameData::RemovePickup(const std::shared_ptr<Pickup>& pickup)
{
	pickups.erase(remove_if(begin(pickups), end(pickups), [=](const weak_ptr<GameObject>& obj)
	{
		return IsSameId(obj, pickup);
	}), pickups.end());

	RemoveGameObject(pickup);
}

void GameData::RemoveGameObject(const std::shared_ptr<GameObject>& gameObject)
{
	GameObjects.erase(remove_if(begin(GameObjects), end(GameObjects), [=](const weak_ptr<GameObject>& obj)
	{
		return IsSameId(obj, gameObject);
	}), GameObjects.end());
}

void GameData::RemoveExpiredReferences()
{
	GameObjects.erase(remove_if(begin(GameObjects), end(GameObjects),
	                            [&](const weak_ptr<GameObject>& obj) { return obj.expired(); }), end(GameObjects));
}


void GameData::AddEnemy(const std::shared_ptr<Enemy> enemy)
{
	if (std::find_if(enemies.begin(), enemies.end(), [&](const std::weak_ptr<GameObject>& gameObject)
	{
		return !gameObject.expired() && gameObject.lock()->Id == enemy->Id;
	}) == enemies.end())
	{
		enemies.push_back(enemy);
		AddGameObject(enemy);
	}
}

void GameData::RemoveEnemy(const std::shared_ptr<Enemy>& enemy)
{
	enemies.erase(remove_if(begin(enemies), end(enemies), [=](const weak_ptr<GameObject>& obj)
	{
		return IsSameId(obj, enemy);
	}), enemies.end());

	RemoveGameObject(enemy);
}

bool GameData::IsSameId(const weak_ptr<GameObject>& obj, const std::shared_ptr<GameObject>& other)
{
	return !obj.expired() && obj.lock()->Id == other->Id;
}

std::shared_ptr<Room> GameData::GetRoomByIndex(const int roomNumber)
{
	return rooms[roomNumber].lock();
}

std::shared_ptr<Player> GameData::GetPlayer() const
{
	return dynamic_pointer_cast<Player>(player.lock());
}

GameData* GameData::Get()
{
	if (instance == nullptr) { instance = new GameData(); }
	return instance;
}

void GameData::AddPickup(const std::shared_ptr<Pickup> pickup)
{
	if (std::find_if(pickups.begin(), pickups.end(), [&](const std::weak_ptr<GameObject>& gameObject)
	{
		return !gameObject.expired() && gameObject.lock()->Id == pickup->Id;
	}) == pickups.end())
	{
		pickups.push_back(pickup);
	}
	AddGameObject(pickup);
	if (!pickups.empty()) { SetGameWon(false); }
}

void GameData::AddGameObject(const std::shared_ptr<GameObject>& gameObject)
{
	if (std::find_if(GameObjects.begin(), GameObjects.end(), [=](const weak_ptr<GameObject>& obj)
	{
		return IsSameId(obj, gameObject);
	}) == GameObjects.end())
	{
		GameObjects.push_back(gameObject);
	}
}
