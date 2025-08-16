#pragma once
#include "EventNumber.h"
#include "events/Event.h"

namespace mazer
{

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
}
