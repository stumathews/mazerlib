#include "CharacterBuilder.h"
#include "GameData.h"
#include "GameObjectMoveStrategy.h"
#include "RoomGenerator.h"
#include "gtest/gtest.h"
#include "cppgamelib/character/Movement.h"
#include "cppgamelib/objects/GameObjectFactory.h"
#include "cppgamelib/resource/ResourceManager.h"
#include <cppgamelib/character/Movement.h>

#include "Level.h"
#include "Room.h"
#include <cppgamelib/asset/SpriteAsset.h>

#include "RoomInfo.h"

using namespace mazer;

class GameObjectMoveStrategyTests : public testing::Test 
{
 protected:

	void SetUp() override
	{
		gamelib::ResourceManager::Get()->Initialize("Resources.xml");
		level = std::make_shared<Level>();
		room = std::make_shared<mazer::Room>("MyRoom", "Room", 0, 0, 0, 100, 100);
		
		GameData::Get()->AddRoom(room);

		const auto enemyName = "beastie01";
		spriteAsset = std::dynamic_pointer_cast<gamelib::SpriteAsset>(gamelib::ResourceManager::Get()->GetAssetInfo(188));

		const auto positionInRoom = room->GetCenter(spriteAsset->Dimensions);
		animatedSprite = gamelib::GameObjectFactory::Get().BuildSprite(
			enemyName, "Enemy", spriteAsset, positionInRoom, true);

		gameObject = std::make_shared<Enemy>(enemyName, "Enemy", positionInRoom, true, room, animatedSprite, gamelib::Direction::Down, level);

	}

	void TearDown() override
	{
		GameData::Get()->GameObjects.clear();
		GameData::Get()->RemoveRoom(room);
	}

	std::shared_ptr<Level> level;
	std::shared_ptr<mazer::Room> room;
	std::shared_ptr<gamelib::GameObject> gameObject;
	std::shared_ptr<gamelib::SpriteAsset> spriteAsset;
	std::shared_ptr<gamelib::AnimatedSprite> animatedSprite;
};

TEST_F(GameObjectMoveStrategyTests, BasicPositionChangedTest)
{
	const auto initialPosition = gameObject->Position;

	// This will be the move strategy that will act upon the game object
	GameObjectMoveStrategy moveStrategy(gameObject, std::make_shared<mazer::RoomInfo>(room));

	// When using the move strategy to move right by 25 pixels...
	moveStrategy.MoveGameObject(std::make_shared<gamelib::Movement>(gamelib::Direction::Right, 25));

	// Ensure we move up by 25 pixels only
	EXPECT_EQ(gameObject->Position.GetX(), initialPosition.GetX() + 25);
	EXPECT_EQ(gameObject->Position.GetY(), initialPosition.GetY());

	// When moving left by 25 pixels
	moveStrategy.MoveGameObject(std::make_shared<gamelib::Movement>(gamelib::Direction::Left, 25));

	// Ensure we only move left by 25 pixels
	EXPECT_EQ(gameObject->Position.GetX(), initialPosition.GetX());
	EXPECT_EQ(gameObject->Position.GetY(), initialPosition.GetY());

	// When moving 25 pixels up
	moveStrategy.MoveGameObject(std::make_shared<gamelib::Movement>(gamelib::Direction::Up, 25));

	// Ensure we only move 25 pixels up
	EXPECT_EQ(gameObject->Position.GetX(), initialPosition.GetX());
	EXPECT_EQ(gameObject->Position.GetY(), initialPosition.GetY() - 25);

	// When moving 25 pixels down
	moveStrategy.MoveGameObject(std::make_shared<gamelib::Movement>(gamelib::Direction::Down, 25));

	// Ensure we are only moving 25 pixels down
	EXPECT_EQ(gameObject->Position.GetX(), initialPosition.GetX());
	EXPECT_EQ(gameObject->Position.GetY(), initialPosition.GetY());
}
