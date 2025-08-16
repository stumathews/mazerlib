#include <cppgamelib/events/EventManager.h>
#include <gtest/gtest.h>

#include "CharacterBuilder.h"
#include "Level.h"
#include "Room.h"
#include "cppgamelib/events/AddGameObjectToCurrentSceneEvent.h"
#include "cppgamelib/objects/GameObjectFactory.h"
#include "cppgamelib/resource/ResourceManager.h"

class LevelTesting : public testing::Test 
{
public:

	void SetUp() override
	{
		gamelib::ResourceManager::Get()->Initialize("Resources.xml");
		TheLevel = std::make_shared<mazer::Level>("Level1.xml");
		RoomPtr = std::make_shared<mazer::Room>("MyRoom", "Room", 0, 0, 0, 100, 100);
	}

	void TearDown() override
	{
		gamelib::EventManager::Get()->Reset();
	}

	std::shared_ptr<mazer::Level> TheLevel = nullptr;
	std::shared_ptr<mazer::Room> RoomPtr = nullptr;
	
};

TEST_F(LevelTesting, Test_LoadsOK)
{
	EXPECT_TRUE(TheLevel->EventSubscriptions.empty()) << "Level should not does not subscribe to any events";
	EXPECT_NO_THROW(TheLevel->Load());
	EXPECT_EQ(TheLevel->Rooms.size(), 100) << "level is not the right number of rooms";
	EXPECT_EQ(TheLevel->Pickups.size(), 12) << "level is not the right number of pickups";
	EXPECT_EQ(TheLevel->Enemies.size(), 0) << "level is not the right number of Enemies";
	EXPECT_EQ(TheLevel->NumCols, 10) << "level is not the right number of columns";
	EXPECT_EQ(TheLevel->NumRows, 10) << "level is not the right number of rows";
	EXPECT_EQ(TheLevel->ScreenWidth, 1024) << "Screen width is incorrect";
	EXPECT_EQ(TheLevel->ScreenHeight, 768) << "Screen width is incorrect";
	EXPECT_FALSE(TheLevel->IsAutoLevel()) << "level should not be auto created level";
	EXPECT_FALSE(TheLevel->IsAutoPopulatePickups()) << "Pickups should not be created automatically";
}

TEST_F(LevelTesting, Test_GetRoom)
{
	TheLevel->Load();

	// Ensure that rooms that do not exist return null
	EXPECT_EQ(TheLevel->GetRoom(1,TheLevel->NumCols+1), nullptr);
	EXPECT_EQ(TheLevel->GetRoom(TheLevel->NumRows + 1, 1), nullptr);

	// Spot check that we are getting the right rooms back
	EXPECT_EQ(TheLevel->GetRoom(1,1)->GetRoomNumber(), 0);	
	EXPECT_EQ(TheLevel->GetRoom(1,5)->GetRoomNumber(), 4);
	EXPECT_EQ(TheLevel->GetRoom(2,2)->GetRoomNumber(), 11);
	EXPECT_EQ(TheLevel->GetRoom(3,3)->GetRoomNumber(), 22);	
	EXPECT_EQ(TheLevel->GetRoom(1, 10)->GetRoomNumber(), 9);
	EXPECT_EQ(TheLevel->GetRoom(5, 9)->GetRoomNumber(), 48);
	EXPECT_EQ(TheLevel->GetRoom(7, 6)->GetRoomNumber(), 65);
	EXPECT_EQ(TheLevel->GetRoom(10, 8)->GetRoomNumber(), 97);
	EXPECT_EQ(TheLevel->GetRoom(10, 10)->GetRoomNumber(), 99);
}
TEST_F(LevelTesting, Test_AddGameObjectToScene)
{
	TheLevel->Load();

	const auto enemy = mazer::CharacterBuilder::BuildEnemy("MyEnemy01", RoomPtr, 18, gamelib::Direction::Down, TheLevel);
	
	TheLevel->AddGameObjectToScene(enemy);

	// Ensure it raises the correct event
	EXPECT_EQ(gamelib::EventManager::Get()->GetEvents().back()->Id, gamelib::AddGameObjectToCurrentSceneEventId);		
}