
#include <asset/SpriteAsset.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>
#include <gtest/gtest.h>

#include "CharacterBuilder.h"
#include "GameData.h"
#include "Level.h"
#include "Room.h"
#include "cppgamelib/resource/ResourceManager.h"
#include "Pickup.h"

class PickupTests : public testing::Test 
{
 protected:

	void SetUp() override
	{
		gamelib::ResourceManager::Get()->Initialize("Resources.xml");
		level = std::make_shared<mazer::Level>();
		room = std::make_shared<mazer::Room>("MyRoom", "Room", 0, 0, 0, 100, 100);
		spriteAsset = std::dynamic_pointer_cast<gamelib::SpriteAsset>(
			gamelib::ResourceManager::Get()->GetAssetInfo(pickupAssetId));
		
		pickup = std::make_shared<mazer::Pickup>(
			pickupName, "Pickup", room->GetCenter(spriteAsset->Dimensions), true,
			room->GetRoomNumber(), spriteAsset);
	}

	void TearDown() override
	{
	}

	std::string pickupName {"MyPickup01"};
	std::shared_ptr<mazer::Level> level;
	std::shared_ptr<mazer::Room> room;
	std::shared_ptr<mazer::Pickup> pickup;
	const int pickupAssetId {19};
	std::shared_ptr<gamelib::SpriteAsset> spriteAsset;
};

TEST_F(PickupTests, Construction)
{
	EXPECT_EQ(pickup->GetGameObjectType(), gamelib::GameObjectType::pickup);
	EXPECT_EQ(pickup->GetSubscriberName(), pickupName);
	EXPECT_EQ(pickup->GetName(), pickupName);
	EXPECT_EQ(pickup->RoomNumber, room->GetRoomNumber());
	EXPECT_THAT(pickup->IntProperties, testing::IsEmpty());
	EXPECT_THAT(pickup->Asset->Uid, testing::Eq(pickupAssetId));
}

TEST_F(PickupTests, Initialize)
{
	EXPECT_NO_THROW(pickup->Initialize());
}