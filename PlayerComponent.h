#pragma once
#include <character/Component.h>
#include "Room.h"

class Player;

class PlayerComponent : public gamelib::Component
{
public:
	PlayerComponent(const std::string& componentName, Player* player);

	Player* ThePlayer;

	int Room;
};
