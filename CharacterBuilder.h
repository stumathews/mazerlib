#pragma once
#include <memory>
#include "Enemy.h"
#include "Pickup.h"
#include "Player.h"
#include "Room.h"

#include "resource/ResourceManager.h"


namespace gamelib
{
	class GameObject;
}


class CharacterBuilder
{
public:
	static [[nodiscard]] std::shared_ptr<Player> BuildPlayer(const std::string& playerName,
	                                                         const std::shared_ptr<Room>& playerRoom,
	                                                         int playerResourceId,
	                                                         const std::string& nickName);

	static [[nodiscard]] std::shared_ptr<gamelib::Pickup> BuildPickup(const std::string& pickupName,
	                                                                  const std::shared_ptr<Room>& pickupRoom,
	                                                                  int pickupResourceId);

	static [[nodiscard]] std::shared_ptr<Enemy> BuildEnemy(const std::string& enemyName, const std::shared_ptr<Room>& enemyRoom,
	                                                       int enemySpriteResourceId,
	                                                       gamelib::Direction startingDirection,
	                                                       const std::shared_ptr<const Level>&
	                                                       level);
};
