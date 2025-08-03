#pragma once
#include <SDL.h>
#include <geometry/ABCDRectangle.h>
#include "objects/DrawableGameObject.h"
#include "objects/GameObject.h"
#include <events/Event.h>
#include "geometry/Coordinate.h"
#include <geometry/Line.h>
#include <common/aliases.h>

#include "Enemy.h"
#include "geometry/Side.h"

class Room final : public gamelib::DrawableGameObject, public std::enable_shared_from_this<Room>
{
public:
	Room(const std::string& name, const std::string& type, int number, int x, int y, int width, int height,
	     bool fill = false);

	bool IsWalled(gamelib::Side wall) const;
	bool HasTopWall() const;
	bool HasBottomWall() const;
	bool HasLeftWall() const;
	bool HasRightWall() const;

	void UpdateInnerBounds();
	void SetupWalls();

	void SetSurroundingRooms(int top_index, int rightIndex, int bottomIndex, int leftIndex,
	                         const std::vector<std::shared_ptr<
		                         Room>>& rooms);
	void RemoveWall(gamelib::Side wall);
	void LogWallRemoval(gamelib::Side wall) const;
	void SetNotWalled(gamelib::Side wall);
	void SetWalled(gamelib::Side wall);
	void AddWall(gamelib::Side wall);
	void RemoveWallZeroBased(gamelib::Side wall);
	void ShouldRoomFill(bool fillMe = false);
	void DrawWalls(SDL_Renderer* renderer) const;
	std::shared_ptr<Room> GetSideRoom(gamelib::Side side);
	void Initialize();
	static void DrawLine(SDL_Renderer* renderer, const gamelib::Line& line);
	void DrawDiagnostics(SDL_Renderer* renderer);
	void LoadSettings() override;
	void Draw(SDL_Renderer* renderer) override;

	gamelib::GameObjectType GetGameObjectType() override { return gamelib::GameObjectType::game_defined; }
	gamelib::ListOfEvents HandleEvent(const std::shared_ptr<gamelib::Event>& event, const unsigned long deltaMs) override;
	gamelib::ListOfEvents& OnPlayerMoved(std::vector<std::shared_ptr<gamelib::Event>>& generatedEvents);
	gamelib::Coordinate<int> GetCenter(int width, int height) const;
	gamelib::Coordinate<int> GetCenter() const;
	gamelib::Coordinate<int> GetCenter(const gamelib::AbcdRectangle& rectangle) const;

	gamelib::Coordinate<int> GetPosition();
	int GetNeighborIndex(gamelib::Side index) const;
	int GetX() const;
	int GetY() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetRoomNumber() const;
	int GetRowNumber(int maxCols) const;
	int GetColumnNumber(int maxCols) const;

	std::string GetName() override;

	SDL_Rect InnerBounds{};

	bool IsTopWalled{};
	bool IsBottomWalled{};
	bool IsRightWalled{};
	bool IsLeftWalled{};

	gamelib::Line TopLine;
	gamelib::Line RightLine;
	gamelib::Line BottomLine;
	gamelib::Line LeftLine;

protected:
	int roomNumber;
	bool walls[4]{};
	gamelib::AbcdRectangle abcd{};
	gamelib::AbcdRectangle& GetAbcdRectangle();
	int topRoomIndex;
	std::shared_ptr<Room> rightRoom;
	std::shared_ptr<Room> leftRoom;
	std::shared_ptr<Room> topRoom;
	std::shared_ptr<Room> bottomRoom;
	int rightRoomIndex;
	int bottomRoomIndex;
	int leftRoomIndex;
	int width;
	int height;

private:
	void UpdateEnemyRoom(const std::shared_ptr<Enemy>& enemy);

public:
	void Update(unsigned long deltaMs) override;

private:
	bool isPlayerWithinRoom = false;
	bool fill = false;
	int innerBoundsOffset;
	bool logWallRemovals;
	bool drawInnerBounds{};
	bool drawHotSpot{};
	bool printDebuggingTextNeighborsOnly{};
	bool printDebuggingText{};	
	bool trackEnemies{};
};
