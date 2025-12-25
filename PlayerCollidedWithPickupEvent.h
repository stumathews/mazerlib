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
			: Event(PlayerCollidedWithPickupEventId), ThePlayer(std::move(player)), ThePickup(std::move(pickup))
		{
		}

		std::shared_ptr<Player> ThePlayer;
		std::shared_ptr<Pickup> ThePickup;
	};
}
