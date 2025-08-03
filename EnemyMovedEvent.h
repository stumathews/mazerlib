#pragma once
#include "Enemy.h"
#include "EventNumber.h"
#include "events/EventId.h"

const gamelib::EventId EnemyMovedEventId(EnemyMoved, "EnemyMovedEvent");

class EnemyMovedEvent final : public gamelib::Event
{
public:
	explicit EnemyMovedEvent(std::shared_ptr<Enemy> enemy)
	: Event(EnemyMovedEventId), Enemy(std::move(enemy))
	{
	}

	std::shared_ptr<Enemy> Enemy;
};
