#include <cppgamelib/utils/Utils.h>
#include <resource/ResourceManager.h>

#include "Pickup.h"
#include "objects/gameobject.h"
#include <asset/SpriteAsset.h>
#include <cppgamelib/events/EventFactory.h>
#include <cppgamelib/events/PlayerMovedEvent.h>
#include "EventNumber.h"
#include "GameData.h"
#include "GameObjectEventFactory.h"
#include "PlayerCollidedWithPickupEvent.h"
#include "SDLCollisionDetection.h"
#include "character/AnimatedSprite.h"
#include "Player.h"
#include "RoomInfo.h"

using namespace std;

namespace mazer
{
	Pickup::Pickup(const std::string& name, const std::string& type, const int x, const int y, const int width,
		const int height, const bool visible, const int inRoomNumber)
		: DrawableGameObject(name, type, gamelib::Coordinate(x, y), visible)
	{
		this->IsVisible = visible;
		this->width = width;
		this->height = height;
		this->RoomNumber = inRoomNumber;
	}

	Pickup::Pickup(const std::string& name, const std::string& type, const gamelib::Coordinate<int> startingPoint,
		// ReSharper disable once CppPassValueParameterByConstReference
		const bool visible, const int inRoomNumber, const std::shared_ptr<gamelib::SpriteAsset> asset)  // NOLINT(performance-unnecessary-value-param)
		: DrawableGameObject(name, type, gamelib::Coordinate(startingPoint.GetX(), startingPoint.GetY()), visible)
	{
		this->IsVisible = visible;
		this->Asset = asset;
		this->width = asset->Dimensions.GetWidth();
		this->height = asset->Dimensions.GetHeight();
		this->RoomNumber = inRoomNumber;
	}

	Pickup::Pickup(const bool visible) : DrawableGameObject(0, 0, visible)
	{
		this->IsVisible = visible;
		this->width = 0;
		this->height = 0;
		this->RoomNumber = 0;
	}

	void Pickup::Initialize()
	{
		SetBounds();

		sprite = gamelib::AnimatedSprite::Create(Position, gamelib::To<gamelib::SpriteAsset>(gamelib::ResourceManager::Get()->GetAssetInfo(Asset->Name)));
		width = sprite->Dimensions.GetWidth();
		height = sprite->Dimensions.GetHeight();
	}

	gamelib::ListOfEvents Pickup::HandleEvent(const std::shared_ptr<gamelib::Event>& event, const unsigned long deltaMs)
	{
		gamelib::ListOfEvents generatedEvents;

		// Player moved?
		if (event->Id.PrimaryId == gamelib::PlayerMovedEventTypeEventId.PrimaryId)
		{
			const auto player = GameData::Get()->GetPlayer();

			// Check if the player collided with us...
			if (IsInSameRoomAsPlayer(player))
			{
				if (SdlCollisionDetection::IsColliding(&player->Bounds, &Bounds))
				{
					// Yes
					generatedEvents.push_back(gamelib::EventFactory::Get()->CreateGenericEvent(FetchedPickupEventId, GetSubscriberName()));
					generatedEvents.push_back(make_shared<PlayerCollidedWithPickupEvent>(player, shared_from_this()));

					// Schedule ourselves to be removed from the game
					generatedEvents.push_back(GameObjectEventFactory::MakeRemoveObjectEvent(shared_from_this()));
				}
			}
		}
		return generatedEvents;
	}

	void Pickup::Draw(SDL_Renderer* renderer)
	{
		sprite->Draw(renderer);
	}

	void Pickup::Update(const unsigned long deltaMs)
	{
		// Move sprite
		sprite->Position.SetX(Position.GetX());
		sprite->Position.SetY(Position.GetY());

		// Update sprite
		sprite->Update(deltaMs);
	}

	gamelib::GameObjectType Pickup::GetGameObjectType()
	{
		return gamelib::GameObjectType::pickup;
	}

	std::string Pickup::GetSubscriberName()
	{
		return Name;
	}

	std::string Pickup::GetName()
	{
		return Name;
	}

	// ReSharper disable once CppPassValueParameterByConstReference
	bool Pickup::IsInSameRoomAsPlayer(const std::shared_ptr<Player> player) const  // NOLINT(performance-unnecessary-value-param)
	{
		return player->CurrentRoom->GetCurrentRoom()->GetRoomNumber() == RoomNumber;
	}

	void Pickup::SetBounds()
	{
		Bounds = CalculateBounds(Position, width, height);
	}
}