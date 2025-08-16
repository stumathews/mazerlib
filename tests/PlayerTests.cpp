#include "pch.h"
#include <Player.h>
#include <memory>
#include <cppgamelib/geometry/Coordinate.h>

using namespace std;
using namespace mazer;

class PlayerTests : public testing::Test 
{
public:

	void SetUp() override
	{
		
		
		Subject = std::make_shared<mazer::Player>(playerName, playerType, gamelib::Coordinate<int>{1,2}, 3, 4, identifier);
		
	}

	void TearDown() override
	{
		
	}
	
	std::shared_ptr<mazer::Player> Subject = nullptr;
	
protected:
	string identifier { "identifier" };
	string playerName { "playerName" };
	string playerType { "Player" };
};

TEST_F(PlayerTests, BasicsSetCorrectly)
{
	EXPECT_EQ(Subject->GetName(), playerName);
	EXPECT_EQ(Subject->Identifier, identifier);
	EXPECT_EQ(Subject->Type, playerType);
	EXPECT_EQ(Subject->Position.GetX(), 1);
	EXPECT_EQ(Subject->Position.GetY(), 2);
	EXPECT_EQ(Subject->Bounds.w, 3);
	EXPECT_EQ(Subject->Bounds.h, 4);
	EXPECT_EQ(Subject->Bounds.x, 1);
	EXPECT_EQ(Subject->Bounds.y, 2);
	EXPECT_EQ(Subject->GetHeight(), 4);
	EXPECT_EQ(Subject->GetWidth(), 3);
	
}

TEST_F(PlayerTests, Basic)
{
	Player player("player1", "Player", gamelib::Coordinate<int>(0,0), 0, 0, "player1");
	EXPECT_EQ(player.Position.GetX(), 0);
	EXPECT_EQ(player.Position.GetY(), 0);
	EXPECT_EQ(player.Bounds.w, 0);
	EXPECT_EQ(player.Bounds.h, 0);
	EXPECT_EQ(player.Bounds.x, 0);
	EXPECT_EQ(player.Bounds.y, 0);
	EXPECT_EQ(player.GetGameObjectType(), gamelib::GameObjectType::game_defined);		
}

TEST_F(PlayerTests, HandleEvents)
{
	Player player("player1", "Player", gamelib::Coordinate<int>(0, 0), 0, 0, "player1");
}