#pragma once

#include "events/Event.h"
#include "EventNumber.h"
#include <events/EventId.h>

namespace mazer
{
	const gamelib::EventId PlayerCollidedWithPickupEventId(PlayerCollidedWithPickup, "CollidedWithEnemyEvent");

	class PlayerCollidedWithPickupEvent final : public gamelib::Event
	{
	public:
		PlayerCollidedWithPickupEvent(std::shared_ptr<Player> player, std::shared_ptr<Pickup> pickup)
			: Event(PlayerCollidedWithPickupEventId), Player(std::move(player)), Pickup(std::move(pickup))
		{
		}

		std::shared_ptr<Player> Player;
		std::shared_ptr<Pickup> Pickup;
	};
}
