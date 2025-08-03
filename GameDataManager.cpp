#include "GameDataManager.h"
#include <events/Event.h>
#include <objects/GameObject.h>
#include "Room.h"
#include <events/AddGameObjectToCurrentSceneEvent.h>
#include <events/GameObjectEvent.h>
#include "EventNumber.h"
#include "pickup.h"
#include "utils/Utils.h"

using namespace std;
using namespace gamelib;

void GameDataManager::Initialize(const bool isNetworkGame = false)
{
	SubscribeToEvent(AddGameObjectToCurrentSceneEventId);
	SubscribeToEvent(GameObjectTypeEventId);
	GameData::Get()->IsNetworkGame = isNetworkGame;
	GameData::Get()->IsGameDone = false;
	GameData::Get()->IsNetworkGame = false;
	GameData::Get()->CanDraw = true;
	GameWorldData.CanDraw = GameData::Get()->CanDraw;
	GameWorldData.IsNetworkGame = GameData::Get()->IsNetworkGame;
	GameWorldData.IsGameDone = GameData::Get()->IsGameDone;
}

GameDataManager::GameDataManager()
{
	eventManager = EventManager::Get();
	eventFactory = EventFactory::Get();
}

GameDataManager* GameDataManager::Get()
{
	if (instance == nullptr) { instance = new GameDataManager(); }
	return instance;
}

GameDataManager::~GameDataManager()
{
	instance = nullptr;
}

ListOfEvents GameDataManager::HandleEvent(const std::shared_ptr<Event>& event, const unsigned long deltaMs)
{
	if (event->Id == AddGameObjectToCurrentSceneEventId)
	{
		AddToGameData(To<AddGameObjectToCurrentSceneEvent>(event));
	}

	if (event->Id == GameObjectTypeEventId)
	{
		RemoveFromGameData(To<GameObjectEvent>(event));
	}
	return {};
}

inline std::string GameDataManager::GetSubscriberName()
{
	return "GameDataManager";
}

void GameDataManager::AddToGameData(const shared_ptr<AddGameObjectToCurrentSceneEvent>& event) const
{
	const auto gameObject = To<AddGameObjectToCurrentSceneEvent>(event)->GetGameObject();

	if (gameObject->GetGameObjectType() == GameObjectType::game_defined)
	{
		if (gameObject->Type == "Room")
		{
			GameData()->AddRoom(To<Room>(gameObject));
		}
		if (gameObject->Type == "Enemy")
		{
			GameData()->AddEnemy(To<Enemy>(gameObject));
		}
	}
	else
	{
		if (gameObject->GetGameObjectType() == GameObjectType::pickup)
		{
			GameData()->AddPickup(To<Pickup>(gameObject));
		}
	}

	GameData()->AddGameObject(gameObject);
}

void GameDataManager::RemoveFromGameData(const std::shared_ptr<GameObjectEvent>& event)
{
	if(event->Context == GameObjectEventContext::Remove)
	{
		RemoveGameObject(event->Object);
	}

	if (GameData::Get()->CountPickups() == 0 && !GameData::Get()->IsGameWon())
	{
		GameData::Get()->SetGameWon(true);
		eventManager->RaiseEvent(EventFactory::Get()->CreateGenericEvent(GameWonEventId, GetSubscriberName()), this);
	}
}

void GameDataManager::RemoveGameObject(const std::shared_ptr<GameObject>& gameObject) const
{
	if (gameObject->Type == "Room")
	{
		GameData::Get()->RemoveRoom(To<Room>(gameObject));
	}

	if (gameObject->Type == "Pickup")
	{
		GameData::Get()->RemovePickup(To<Pickup>(gameObject));
	}

	if (gameObject->Type == "Enemy")
	{
		GameData::Get()->RemoveEnemy(To<Enemy>(gameObject));
	}

	GameData::Get()->RemoveGameObject(gameObject);
	GameData::Get()->RemoveExpiredReferences();
	eventManager->Unsubscribe(gameObject->GetSubscriberId());
}

GameDataManager* GameDataManager::instance = nullptr;