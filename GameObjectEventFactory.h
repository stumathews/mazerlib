#pragma once
#include <memory>
#include <events/GameObjectEvent.h>

#include "events/AddGameObjectToCurrentSceneEvent.h"

class GameObjectEventFactory
{
public:
	static std::shared_ptr<gamelib::GameObjectEvent> MakeRemoveObjectEvent(
		const std::shared_ptr<gamelib::GameObject>& target)
	{
		return std::make_shared<gamelib::GameObjectEvent>(target, gamelib::GameObjectEventContext::Remove);
	}

	static std::shared_ptr<gamelib::AddGameObjectToCurrentSceneEvent> MakeAddGameObjectToSceneEvent(
		const std::shared_ptr<gamelib::GameObject>& obj)
	{
		return std::make_shared<gamelib::AddGameObjectToCurrentSceneEvent>(obj);
	}
};
