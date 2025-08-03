#pragma once
#ifndef GAMEDATAMANAGER_H
#define GAMEDATAMANAGER_H

#include <events/EventSubscriber.h>
#include "GameData.h"
#include <events/EventManager.h>
#include <events/EventFactory.h>


namespace gamelib
{
	class GameObjectEvent;
}

// Wrapper and access to GameData
class GameDataManager final : public gamelib::EventSubscriber
{
public:
	static GameDataManager* Get();

	GameDataManager(const GameDataManager& other) = delete;
	GameDataManager(const GameDataManager&& other) = delete;
	GameDataManager& operator=(const GameDataManager& other) = delete;
	GameDataManager& operator=(const GameDataManager&& other) = delete;

	~GameDataManager() override;

	gamelib::ListOfEvents HandleEvent(const std::shared_ptr<gamelib::Event>& event, const unsigned long deltaMs) override;
	std::string GetSubscriberName() override;
	void Initialize(bool isNetworkGame);
	
	static GameData* GameData() { return GameData::Get(); }
	gamelib::GameWorldData GameWorldData {};
protected:
	static GameDataManager* instance;
private:
	GameDataManager();
	void AddToGameData(const std::shared_ptr<gamelib::AddGameObjectToCurrentSceneEvent>& event) const;
	void RemoveFromGameData(const std::shared_ptr<gamelib::GameObjectEvent>& event);
	void RemoveGameObject(const std::shared_ptr<gamelib::GameObject>& gameObject) const;

	gamelib::EventManager* eventManager;
	gamelib::EventFactory* eventFactory;

};

#endif 