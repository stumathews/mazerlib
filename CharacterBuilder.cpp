#include "CharacterBuilder.h"
#include "Enemy.h"
#include "GameData.h"
#include "GameObjectMoveStrategy.h"
#include "asset/SpriteAsset.h"
#include "common/constants.h"
#include "events/PlayerMovedEvent.h"
#include "objects/GameObjectFactory.h"
#include <utils/Utils.h>

using namespace gamelib;

std::shared_ptr<Player> CharacterBuilder::BuildPlayer(const std::string& playerName,
                                                      const std::shared_ptr<Room>& playerRoom,
                                                      const int playerResourceId, const std::string& nickName)
{
	// The player's sprite sheet
	const auto spriteAsset = To<SpriteAsset>(ResourceManager::Get()->GetAssetInfo(playerResourceId));

	const auto positionInRoom = playerRoom->GetCenter(spriteAsset->Dimensions);

	// Build player's sprite
	const auto animatedSprite = GameObjectFactory::BuildSprite(
		playerName,
		"Player",
		spriteAsset,
		positionInRoom,
		true);

	// Build player
	auto player = std::make_shared<Player>(
		playerName,
		"Player", 
		playerRoom,
		spriteAsset->Dimensions, 
		nickName);

	// Initialize player
	player->LoadSettings();
	player->SetMoveStrategy(std::make_shared<GameObjectMoveStrategy>(player, player->CurrentRoom));
	player->SetTag(gamelib::PlayerTag);
	player->SetSprite(animatedSprite);
	player->IntProperties["Health"] = 100;

	// We keep a reference to track of the player globally
	GameData::Get()->player = player;

	return player;
}

std::shared_ptr<Enemy> CharacterBuilder::BuildEnemy(const std::string& enemyName, const std::shared_ptr<Room>& enemyRoom,
                                                    const int enemySpriteResourceId, Direction startingDirection,
                                                    const std::shared_ptr<const Level>& level)
{
	// A enemy's sprite asset
	const auto spriteAsset = To<SpriteAsset>(ResourceManager::Get()->GetAssetInfo(enemySpriteResourceId));

	const auto positionInRoom = enemyRoom->GetCenter(spriteAsset->Dimensions);

	// Build sprite
	const auto enemyAnimatedSprite = GameObjectFactory::BuildSprite(
		enemyName, 
	"Enemy", 
		spriteAsset, 
		positionInRoom, 
		true);

	auto enemy = std::make_shared<Enemy>(enemyName,
		"Enemy",
		positionInRoom,
		true,
		enemyRoom,
		enemyAnimatedSprite,
		startingDirection,
		level);

	return enemy;
}


std::shared_ptr<Pickup> CharacterBuilder::BuildPickup(const std::string& pickupName,
                                                      const std::shared_ptr<Room>& pickupRoom,
                                                      const int pickupResourceId)
{	
	const auto pickupSpriteSheet = To<SpriteAsset>(ResourceManager::Get()->GetAssetInfo(pickupResourceId));

	const auto positionInRoom = pickupRoom->GetCenter(pickupSpriteSheet->Dimensions);

	auto pickup = std::make_shared<Pickup>(pickupName, "Pickup", 
		positionInRoom,
		true,
		pickupRoom->GetRoomNumber(), 
		pickupSpriteSheet);

	pickup->Initialize();
	pickup->LoadSettings();
	pickup->SubscribeToEvent(PlayerMovedEventTypeEventId);
	return pickup;
}
