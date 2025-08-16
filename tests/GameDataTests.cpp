#include "CharacterBuilder.h"
#include "GameData.h"
#include "Level.h"
#include "Player.h"
#include "Room.h"
#include "gtest/gtest.h"

using namespace mazer;

class GameDataTests : public testing::Test 
{
 protected:

	void SetUp() override
	{
		gamelib::ResourceManager::Get()->Initialize("Resources.xml");
		level = std::make_shared<Level>();
		room = std::make_shared<Room>("MyRoom", "Room", 1, 0, 0, 0, 0);
	}

	void TearDown() override
	{
		GameData::Get()->GameObjects.clear();
	}

	std::shared_ptr<Level> level;
	std::shared_ptr<Room> room;
		int myResourceId = 188;
};


TEST_F(GameDataTests, GameData_Initial_State_Is_Correct)
{	
	EXPECT_TRUE(GameData::Get()->GameObjects.empty());	
	EXPECT_FALSE(GameData::Get()->IsGameWon());
	EXPECT_FALSE(GameData::Get()->IsNetworkGame);
	EXPECT_FALSE(GameData::Get()->IsGameDone);
}

TEST_F(GameDataTests, Add_Remove_Enemy)
{	
	const auto enemy = CharacterBuilder::BuildEnemy("MyEnemy", room, myResourceId, gamelib::Direction::Down, level);

	// When adding the same enemy...
	GameData::Get()->AddEnemy(enemy);
	GameData::Get()->AddEnemy(enemy);

	// Only one enemy should be added
	EXPECT_EQ(GameData::Get()->Enemies().size(), 1);
	EXPECT_EQ(GameData::Get()->GameObjects.size(), 1);

	// When removing the enemy
	GameData::Get()->RemoveEnemy(enemy);

	// The enemy should be removed from both the game Objects and the Enemies
	EXPECT_TRUE(GameData::Get()->Enemies().empty());
	EXPECT_TRUE(GameData::Get()->GameObjects.empty());
}

TEST_F(GameDataTests, Add_Remove_Pickup)
{
	const auto pickup = CharacterBuilder::BuildPickup("MyPickup", room, myResourceId);
	GameData::Get()->AddPickup(pickup);
	GameData::Get()->AddPickup(pickup);
	EXPECT_EQ(GameData::Get()->CountPickups(), 1);
	EXPECT_EQ(GameData::Get()->GameObjects.size(), 1);

	GameData::Get()->RemovePickup(pickup);

	EXPECT_EQ(GameData::Get()->CountPickups(), 0);
	EXPECT_TRUE(GameData::Get()->GameObjects.empty());
}


TEST_F(GameDataTests, Add_Remove_Room)
{
	GameData::Get()->AddRoom(room);
	GameData::Get()->AddRoom(room);
	EXPECT_EQ(GameData::Get()->GetRoomByIndex(room->GetRoomNumber()),  room);
	EXPECT_EQ(GameData::Get()->GameObjects.size(), 1);
	GameData::Get()->RemoveRoom(room);
	EXPECT_TRUE(GameData::Get()->GameObjects.empty());
	EXPECT_EQ(GameData::Get()->GetRoomByIndex(room->GetRoomNumber()), nullptr );
}

TEST_F(GameDataTests, Add_Remove_GameObject)
{
	GameData::Get()->AddGameObject(room);
	GameData::Get()->AddGameObject(room);
	EXPECT_EQ(GameData::Get()->GameObjects.size(), 1);
	GameData::Get()->RemoveGameObject(room);
	EXPECT_TRUE(GameData::Get()->GameObjects.empty());
	EXPECT_EQ(GameData::Get()->GetRoomByIndex(room->GetRoomNumber()), nullptr );
}

TEST_F(GameDataTests, GameData_Player)
{
	const auto player = CharacterBuilder::BuildPlayer("MyPlayer", room, myResourceId, "StuNick");
	GameData::Get()->player = player;
	EXPECT_EQ(GameData::Get()->GetPlayer()->Identifier, player->Identifier);
	EXPECT_FALSE(GameData::Get()->player.expired());
}

TEST_F(GameDataTests, SetGameWon)
{
	GameData::Get()->SetGameWon(true);

	EXPECT_TRUE(GameData::Get()->IsGameWon());
	GameData::Get()->SetGameWon(false);

	EXPECT_FALSE(GameData::Get()->IsGameWon());
}

TEST_F(GameDataTests, RemoveExpiredReferences)
{
	const auto pickup = CharacterBuilder::BuildPickup("MyPickup", room, myResourceId);
	auto enemy = CharacterBuilder::BuildEnemy("MyEnemy", room, myResourceId, gamelib::Direction::Down, level);

	GameData::Get()->AddEnemy(enemy);
	GameData::Get()->AddPickup(pickup);
	GameData::Get()->AddGameObject(room);

	EXPECT_EQ(GameData::Get()->GameObjects.size(), 3);

	// Make one of the game objects go away..
	enemy = nullptr;
	EXPECT_EQ(GameData::Get()->GameObjects.size(), 3);

	GameData::Get()->RemoveExpiredReferences();

	// This should remove the expired reference to the game object from the list of game objects
	EXPECT_EQ(GameData::Get()->GameObjects.size(), 2);

	// Also removing the Pickup should do the same
	GameData::Get()->RemovePickup(pickup);

	GameData::Get()->RemoveExpiredReferences();

	EXPECT_EQ(GameData::Get()->GameObjects.size(), 1);
}