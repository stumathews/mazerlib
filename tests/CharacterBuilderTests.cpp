#include <memory>
#include <asset/asset.h>
#include <asset/SpriteAsset.h>
#include <character/AnimatedSprite.h>
#include <cppgamelib/events/EventManager.h>
#include <geometry/ABCDRectangle.h>

#include "CharacterBuilder.h"
#include "EventNumber.h"
#include "GameData.h"
#include "cppgamelib/common/constants.h"
#include "cppgamelib/events/PlayerMovedEvent.h"
#include "gtest/gtest.h"
#include "Level.h"
#include "Room.h"
#include "Enemy.h"
#include "RoomInfo.h"
#include "Pickup.h"
#include "Player.h"

class CharacterBuilderTests : public testing::Test 
{
 protected:

	void SetUp() override
	{
		gamelib::ResourceManager::Get()->Initialize("Resources.xml");
		expectedAssetDimensions = gamelib::AbcdRectangle(0,0,32,32);
		level = std::make_shared<mazer::Level>();
		room = std::make_shared<mazer::Room>("MyRoom", "Room", roomNumber, roomX, roomY, roomWidth, roomHeight);
	}

	void TearDown() override
	{
	  gamelib::EventManager::Get()->ClearSubscribers();
	}

	void DoBasicNpcTests(const std::shared_ptr<gamelib::Npc> npc) const;
	static void DoBasicDrawableObjectTests(const std::shared_ptr<gamelib::DrawableGameObject>& drawableGameObject);
	static void DoBasicGameObjectTests(const std::shared_ptr<gamelib::GameObject>& gameObject, const std::string& type);

	int roomX = 0;
	int roomY = 0;
	int roomNumber = 12;
	int roomWidth = 100;
	int roomHeight = 100;
	gamelib::AbcdRectangle expectedAssetDimensions = {};
	int myResourceId = 188;
	gamelib::Direction expectedStartingDirection = gamelib::Direction::Down;
	std::shared_ptr<mazer::Level> level;
	std::shared_ptr<mazer::Room> room;
};


TEST_F(CharacterBuilderTests, BuildEnemyWorks)
{
	constexpr auto expectedNpcType = "Enemy";	
	
	// When building a new Enemy
	const auto enemy = mazer::CharacterBuilder::BuildEnemy("MyEnemy", room, myResourceId, expectedStartingDirection, level);

	// Ensure it is constructed correctly
	DoBasicGameObjectTests(enemy, expectedNpcType);
	DoBasicNpcTests(enemy);

	EXPECT_TRUE(enemy->Position == room->GetCenter(expectedAssetDimensions));
	EXPECT_TRUE(enemy->IsVisible);
	EXPECT_TRUE(enemy->GetDirection() == expectedStartingDirection);
	EXPECT_TRUE(enemy->CurrentLevel == level);
	EXPECT_TRUE(enemy->CurrentRoom->TheRoom == room);

	// When initializing the enemy object...
	enemy->Initialize();

	// Ensure the initialization is correct
	EXPECT_TRUE(enemy->SubscribesTo(gamelib::PlayerMovedEventTypeEventId));

}

TEST_F(CharacterBuilderTests, BuildPickupWorks)
{
	const auto pickupName = "myPickup";
	const auto pickup = mazer::CharacterBuilder::BuildPickup(pickupName, room, myResourceId);

	DoBasicGameObjectTests(pickup, "Pickup");
	DoBasicDrawableObjectTests(pickup);

	EXPECT_TRUE(pickup->RoomNumber == roomNumber);
	EXPECT_TRUE(pickup->GetName() == pickupName);
	EXPECT_TRUE(pickup->IsVisible);
	EXPECT_TRUE(pickup->IsActive);
	EXPECT_TRUE(pickup->Position == room->GetCenter(expectedAssetDimensions));
	EXPECT_TRUE(pickup->Asset->Uid == myResourceId);
	EXPECT_TRUE(pickup->SubscribesTo(gamelib::PlayerMovedEventTypeEventId));
}

TEST_F(CharacterBuilderTests, BuildPlayerWorks)
{ 
	const auto expectedPlayerName = "MyPlayer";
	const auto expectedPlayerNick = "Stu";

	// When constructing a player...
	const auto player = mazer::CharacterBuilder::BuildPlayer(expectedPlayerName, room, myResourceId, expectedPlayerNick);

	// Ensure its constructed correctly
	EXPECT_TRUE(player->GetName() == expectedPlayerName);
	EXPECT_TRUE(player->Identifier == expectedPlayerNick);
	EXPECT_TRUE(player->Type == "Player");
	//EXPECT_TRUE(player->Position == room->GetCenter(expectedAssetDimensions));
	EXPECT_TRUE(player->GetTag() == gamelib::PlayerTag);
	EXPECT_TRUE(player->IntProperties["Health"] == 100);
	EXPECT_TRUE(mazer::GameData::Get()->player.lock()->Id == player->Id);
}


void CharacterBuilderTests::DoBasicNpcTests(const std::shared_ptr<gamelib::Npc> npc) const
{
	EXPECT_TRUE(npc->Sprite->Asset->Uid == myResourceId);
	EXPECT_TRUE(npc->Sprite->Asset->Dimensions == expectedAssetDimensions);
	
}

void CharacterBuilderTests::DoBasicDrawableObjectTests(const std::shared_ptr<gamelib::DrawableGameObject>& drawableGameObject)
{
}

void CharacterBuilderTests::DoBasicGameObjectTests(const std::shared_ptr<gamelib::GameObject>& gameObject, const std::string& type)
{
	EXPECT_TRUE(gameObject->Type == type);
}