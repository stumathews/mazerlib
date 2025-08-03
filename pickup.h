#pragma once

#include "Room.h"
#include "objects/DrawableGameObject.h"
#include <character/AnimatedSprite.h>
#include <geometry/Coordinate.h>

#include "asset/SpriteAsset.h"

namespace gamelib
{
	class Pickup final : public DrawableGameObject, public std::enable_shared_from_this<Pickup>
	{
	public:
		Pickup(const std::string& name, const std::string& type, const int x, const int y, const int width,
		       const int height, const bool visible, const int inRoomNumber);

		Pickup(const std::string& name, const std::string& type, const Coordinate<int> startingPoint, const bool visible,
		       const int inRoomNumber, const std::shared_ptr<SpriteAsset> asset);

		explicit Pickup(const bool visible);

		GameObjectType GetGameObjectType() override;

		std::string GetSubscriberName() override;
		std::string GetName() override;
		bool IsInSameRoomAsPlayer(std::shared_ptr<Player> player) const;
		ListOfEvents HandleEvent(const std::shared_ptr<Event>& event, const unsigned long deltaMs) override;

		void Initialize();
		void Draw(SDL_Renderer* renderer) override;
		void Update(unsigned long deltaMs) override;

		int RoomNumber;
		std::shared_ptr<Asset> Asset;

	protected:
		void SetBounds();

	private:
		int width;
		int height;
		std::shared_ptr<AnimatedSprite> sprite;
	};
}
