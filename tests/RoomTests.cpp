#include "pch.h"
#include "Room.h"

using namespace mazer;

TEST(RoomTests, Basic)
{
	int number = 1;
	int x = 0;
	int y = 0;
	int width = 10;
	int height = 10;
	bool fill = false;
	Room room("roomName", "Room", number, x, y, width, height, fill);

	// Expect all walls of a room to be walled solid by default
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Top));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Bottom));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Left));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Right));
	EXPECT_TRUE(room.IsActive);
	EXPECT_TRUE(room.IsVisible);
	EXPECT_EQ(room.GetWidth(), width);
	EXPECT_EQ(room.GetHeight(), height);
	EXPECT_EQ(room.GetX(), x);
	EXPECT_EQ(room.GetY(), y);
	EXPECT_FALSE(room.HasGraphic());
	EXPECT_EQ(room.GetGameObjectType(), gamelib::GameObjectType::game_defined);
	EXPECT_EQ(room.GetGraphic(), nullptr);
	EXPECT_FALSE(room.HasColourKey());
	EXPECT_EQ(room.GetColourKey().r, 0);
	EXPECT_EQ(room.GetColourKey().b, 0);
	EXPECT_EQ(room.GetColourKey().g, 0);	
}

TEST(RoomTests, RemoveWalls)
{
	int number = 1;
	int x = 0;
	int y = 0;
	int width = 10;
	int height = 10;
	bool fill = false;
	Room room("NoName","Room", number, x, y, width, height, fill);

	EXPECT_TRUE(room.IsWalled(gamelib::Side::Bottom));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Top));	
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Left));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Right));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Bottom);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Bottom));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Top);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Top));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Left);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Left));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Right);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Right));
}

TEST(RoomTests, AddWalls)
{
	int number = 1;
	int x = 0;
	int y = 0;
	int width = 10;
	int height = 10;
	bool fill = false;
	Room room("noName", "Room", number, x, y, width, height, fill);

	EXPECT_TRUE(room.IsWalled(gamelib::Side::Bottom));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Top));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Left));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Right));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Bottom);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Bottom));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Top);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Top));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Left);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Left));

	// Remove Wall
	room.RemoveWall(gamelib::Side::Right);
	EXPECT_FALSE(room.IsWalled(gamelib::Side::Right));

	room.AddWall(gamelib::Side::Top);
	room.AddWall(gamelib::Side::Right);
	room.AddWall(gamelib::Side::Bottom);
	room.AddWall(gamelib::Side::Left);

	EXPECT_TRUE(room.IsWalled(gamelib::Side::Bottom));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Top));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Left));
	EXPECT_TRUE(room.IsWalled(gamelib::Side::Right));
}