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
			: Event(PlayerCollidedWithEnemyEventId), TheEnemy(std::move(enemy)), ThePlayer(std::move(other))
		{
		}

		std::shared_ptr<Enemy> TheEnemy;
		std::shared_ptr<Player> ThePlayer;
	};
}
