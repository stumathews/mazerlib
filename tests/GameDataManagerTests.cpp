#include "CharacterBuilder.h"
#include "GameData.h"
#include "GameDataManager.h"
#include "gmock/gmock.h"
#include "GameObjectEventFactory.h"
#include "Room.h"
#include "cppgamelib/events/AddGameObjectToCurrentSceneEvent.h"
#include "cppgamelib/events/GameObjectEvent.h"
#include "gtest/gtest.h"
#include "cppgamelib/resource/ResourceManager.h"
#include <memory>
#include <cppgamelib/events/EventManager.h>
#include <cppgamelib/events/AddGameObjectToCurrentSceneEvent.h>
#include <cppgamelib/events/EventFactory.h>

#include "Level.h"
#include "Player.h"
#include "Pickup.h"

using namespace std;
using namespace mazer;

class MockPlayer final : public Player
{
public:
	MockPlayer(const std::string& name, const std::string& type, const gamelib::Coordinate<int>& position, const int width,
	           const int height, const std::string& identifier)
		: Player(name, type, position, width, height, identifier)
	{
	}

	MockPlayer(const std::string& name, const std::string& type, const std::shared_ptr<Room>& playerRoom,
	           const int playerWidth, const int playerHeight, const std::string& identifier)
		: Player(name, type, playerRoom, playerWidth, playerHeight, identifier)
	{
	}
	
  //MOCK_METHOD(void, Update, (float), (override));
  MOCK_METHOD(void, Draw, (SDL_Renderer*), (override));
  MOCK_METHOD(gamelib::GameObjectType, GetGameObjectType, (), (override));
  MOCK_METHOD(std::string, GetName,(), (override));
  MOCK_METHOD(void, LoadSettings,(), (override));
};

class GameDataManagerTests : public testing::Test, public gamelib::EventSubscriber
{
public:
	void SetUp() override
	{
		gamelib::ResourceManager::Get()->Initialize("Resources.xml");
		level = std::make_shared<Level>();
		room = std::make_shared<Room>("MyRoom", "Room", 1, 0, 0, 0, 0);
		subject = GameDataManager::Get();
		player = CreateMockPlayer();
		GameData::Get()->Clear();
		
		gamelib::EventManager::Get()->ClearSubscribers();
	}

	std::vector<std::shared_ptr<gamelib::Event>> HandleEvent(const std::shared_ptr<gamelib::Event>& evt, const unsigned long deltaMs = 0) override
	{
		return {};
	}

	std::string GetSubscriberName() override { return "SubjectTests"; }

	void TearDown() override
	{
		GameData::Get()->GameObjects.clear();
		subject = nullptr;
	}

protected:
	GameDataManager* subject = nullptr;

	const std::string mockPlayerName {"MockPlayer"};
	const std::string playerType {"Player"};
	std::shared_ptr<MockPlayer> player;


	std::shared_ptr<Level> level;
	std::shared_ptr<Room> room;
	int myResourceId = 188;
	
	[[nodiscard]] std::shared_ptr<MockPlayer> CreateMockPlayer() const
	{		
		const auto theRoom = std::make_shared<Room>(mockPlayerName,"Room",0,0,0,0,0, false);
		return make_shared<MockPlayer>(mockPlayerName, playerType, theRoom, 0, 0, mockPlayerName);
	}
};

TEST_F(GameDataManagerTests, Initialize)
{
	subject->Initialize(false);

	EXPECT_TRUE(subject->SubscribesTo(gamelib::AddGameObjectToCurrentSceneEventId));
	EXPECT_TRUE(subject->SubscribesTo(gamelib::GameObjectTypeEventId));
}

TEST_F(GameDataManagerTests, InitialState)
{
	EXPECT_TRUE(subject->GameData());
	EXPECT_TRUE(subject->GetSubscriberName() == "GameDataManager");
}

TEST_F(GameDataManagerTests, Handles_Add_Game_Object_To_Scene_Correctly)
{
	subject->Initialize(false);

	const auto pickup = CharacterBuilder::BuildPickup("MyPickup", room, myResourceId);
	const auto enemy = CharacterBuilder::BuildEnemy("MyEnemy", room, myResourceId, gamelib::Direction::Down, level);
	
	RaiseEvent(std::dynamic_pointer_cast<gamelib::Event>(GameObjectEventFactory::MakeAddGameObjectToSceneEvent(room)));
	RaiseEvent(std::dynamic_pointer_cast<gamelib::Event>(GameObjectEventFactory::MakeAddGameObjectToSceneEvent(pickup)));
	RaiseEvent(std::dynamic_pointer_cast<gamelib::Event>(GameObjectEventFactory::MakeAddGameObjectToSceneEvent(enemy)));
	gamelib::EventManager::Get()->ProcessAllEvents();
	EXPECT_TRUE(GameData::Get()->CountPickups() == 1);
	EXPECT_TRUE(GameData::Get()->Enemies().size() == 1);
	EXPECT_TRUE(GameData::Get()->GetRoomByIndex(room->GetRoomNumber()) == room);
	
	EXPECT_TRUE(GameData::Get()->GameObjects.size() == 3);
}


TEST_F(GameDataManagerTests, Adds_GameObject)
{
	// Put expectations in place that dependencies will be called...
	EXPECT_CALL(*player, GetGameObjectType()).Times(testing::AtLeast(1)).WillRepeatedly(
		testing::Return(gamelib::GameObjectType::game_defined));

	// When receiving an event to add game object to scene
	subject->HandleEvent(std::dynamic_pointer_cast<gamelib::Event>(gamelib::EventFactory::CreateAddToSceneEvent(player)), 0);

	const auto gameObjects = subject->GameData()->GameObjects;
	const auto gameObject = gameObjects[0].lock();

	// Ensure its added to the game data
	EXPECT_EQ(gameObjects.size(), 1) << "Expected 1 game object";

	// Ensure that is the expected game object added.
	EXPECT_EQ(gameObject->Name, mockPlayerName);
	EXPECT_EQ(gameObject->Type, playerType);	
}

TEST_F(GameDataManagerTests, Deletes_GameObject)
{
	// Put expectations in place that dependencies will be called...
	EXPECT_CALL(*player, GetGameObjectType()).Times(testing::AtLeast(1));

	// When receiving an event to add game object to scene, and then getting event to remove it...
	subject->HandleEvent(std::dynamic_pointer_cast<gamelib::Event>(GameObjectEventFactory::MakeAddGameObjectToSceneEvent(player)), 0);
	subject->HandleEvent(std::dynamic_pointer_cast<gamelib::Event>(GameObjectEventFactory::MakeRemoveObjectEvent(player)), 0);
	
	// Ensure its removed from the game data
	EXPECT_EQ(subject->GameData()->GameObjects.size(), 0) << "Expected 0 game object";
}

TEST_F(GameDataManagerTests, Subscriber_Name_Is_Correct)
{
	EXPECT_STREQ(subject->GetSubscriberName().c_str(), "GameDataManager") << "Unexpected Subscriber name";
}
