#pragma once
#ifndef LEVEL_H
#define LEVEL_H
#include <string>
#include <vector>
#include <memory>

#include "Enemy.h"
#include "asset/SpriteAsset.h"
#include "events/EventSubscriber.h"
#include <objects/GameObject.h>

namespace gamelib
{
	class GameObject;
	class Pickup;
}

namespace tinyxml2
{
	class XMLNode;
}

class Player;
class Room;
class Enemy;

class Level final : public gamelib::EventSubscriber, public std::enable_shared_from_this<Level>
{
public:
	explicit Level(const std::string& filename);
	Level();
	void InitializeEnemies();
	void Load();
	gamelib::ListOfEvents HandleEvent(const std::shared_ptr<gamelib::Event>& evt, const unsigned long deltaMs) override;
	std::string GetSubscriberName() override { return "Level"; }
	void InitializePickups(const std::vector<std::shared_ptr<gamelib::Pickup>>& inPickups);
	void AddGameObjectToScene(const std::shared_ptr<gamelib::GameObject>& object);
	std::shared_ptr<gamelib::GameObject> ParseObject(tinyxml2::XMLNode* pObject, const std::shared_ptr<Room>& room) const;
	static void InitializePlayer(const std::shared_ptr<Player>& inPlayer, const std::shared_ptr<gamelib::SpriteAsset>& spriteAsset);
	static std::tuple<std::string, std::string> ParseProperty(tinyxml2::XMLNode* pObjectChild, const std::shared_ptr<gamelib::GameObject>& gameObject);
	std::shared_ptr<Room> GetRoom(int row, int col);	
	[[nodiscard]] bool IsAutoLevel() const { return isAutoLevel || FileName.empty(); }
	[[nodiscard]] bool IsAutoPopulatePickups() const { return isAutoPopulatePickups; }

	std::vector<std::shared_ptr<Room>> Rooms;
	std::vector<std::shared_ptr<gamelib::Pickup>> Pickups;
	std::vector<std::shared_ptr<Enemy>> Enemies;
	std::shared_ptr<Player> Player1;
	std::string FileName;
	int NumCols;
	int NumRows;
	unsigned int ScreenWidth;
	unsigned int ScreenHeight;

private:
	bool isAutoLevel;
	bool isAutoPopulatePickups;
};

#endif
