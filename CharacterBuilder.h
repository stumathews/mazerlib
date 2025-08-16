#pragma once
#include <memory>

#include "resource/ResourceManager.h"


namespace gamelib
{
	enum class Direction;
	class GameObject;
}

namespace mazer
{

	class Room;
	class Player;
	class Level;
	class Enemy;

	class Pickup;

	class CharacterBuilder
	{
	public:
		static [[nodiscard]] std::shared_ptr<Player> BuildPlayer(const std::string& playerName,
			const std::shared_ptr<Room>& playerRoom,
			int playerResourceId,
			const std::string& nickName);

		static [[nodiscard]] std::shared_ptr<mazer::Pickup> BuildPickup(const std::string& pickupName,
			const std::shared_ptr<Room>& pickupRoom,
			int pickupResourceId);

		static [[nodiscard]] std::shared_ptr<Enemy> BuildEnemy(const std::string& enemyName, const std::shared_ptr<Room>& enemyRoom,
			int enemySpriteResourceId,
			gamelib::Direction startingDirection,
			const std::shared_ptr<const Level>&
			level);
	};
}
