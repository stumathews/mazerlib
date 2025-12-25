#pragma once

#include "EventNumber.h"
#include "events/EventId.h"


namespace mazer
{
	const gamelib::EventId EnemyMovedEventId(EnemyMoved, "EnemyMovedEvent");

	class EnemyMovedEvent final : public gamelib::Event
	{
	public:
		explicit EnemyMovedEvent(std::shared_ptr<Enemy> enemy)
			: Event(EnemyMovedEventId), TheEnemy(std::move(enemy))
		{
		}

		std::shared_ptr<Enemy> TheEnemy;
	};
}
