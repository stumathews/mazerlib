#pragma once
#include <character/Component.h>

namespace mazer
{
	class Player;

	class PlayerComponent : public gamelib::Component
	{
	public:
		PlayerComponent(const std::string& componentName, Player* player);

		Player* ThePlayer;

		int Room;
	};
}
