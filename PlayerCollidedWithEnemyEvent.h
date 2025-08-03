#pragma once
#include "Enemy.h"
#include "EventNumber.h"
#include "Player.h"
#include "events/Event.h"

const gamelib::EventId PlayerCollidedWithEnemyEventId(CollidedWithEnemy, "CollidedWithEnemyEvent");

class PlayerCollidedWithEnemyEvent final : public gamelib::Event
{
public:
	explicit PlayerCollidedWithEnemyEvent(std::shared_ptr<Enemy> enemy, std::shared_ptr<Player> other)
		: Event(PlayerCollidedWithEnemyEventId), Enemy(std::move(enemy)), Player(std::move(other))
	{
	}

	std::shared_ptr<Enemy> Enemy;
	std::shared_ptr<Player> Player;
};
