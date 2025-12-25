#include "pch.h"
#include "Room.h"
#include "graphic/RectDebugging.h"
#include <scene/SceneManager.h>
#include "Player.h"
#include <sstream>
#include <character/Hotspot.h>

#include "EventNumber.h"
#include "GameData.h"
#include "GameDataManager.h"
#include "geometry/SideUtils.h"
#include "file/SettingsManager.h"
#include <utils/Utils.h>

#include "Enemy.h"
#include "EnemyMovedEvent.h"
#include "RoomInfo.h"
#include "events/PlayerMovedEvent.h"
#include "file/Logger.h"

using namespace std;
using namespace gamelib;

namespace mazer
{

	Room::Room(const string& name, const string& type, const int number, const int x, const int y, const int width,
		const int height, const bool fill)
		: DrawableGameObject(name, type, Coordinate(x, y), true)
	{
		this->Bounds = { x, y, width, height };
		this->width = width;
		this->height = height;
		this->roomNumber = number;
		this->logWallRemovals = false;
		this->fill = fill;
		this->topRoomIndex = 0;
		this->rightRoomIndex = 0;
		this->bottomRoomIndex = 0;
		this->width = width;
		this->height = height;
		this->leftRoomIndex = 0;
		this->innerBoundsOffset = 0;
		this->abcd = AbcdRectangle(x, y, width, height);

		UpdateInnerBounds(); // We only need to update the bounds once, so do it in the constructor only
		SetupWalls();
	}

	void Room::UpdateInnerBounds()
	{
		this->InnerBounds = SDL_Rect
		{
			Bounds.x + innerBoundsOffset,
			Bounds.y + innerBoundsOffset,
			Bounds.w - innerBoundsOffset * 2,
			Bounds.h - innerBoundsOffset * 2
		};
	}

	void Room::SetupWalls()
	{
		/*
			A(ax,ay)----B(bx,by)
			|                  |
			|                  |
			D(dx,dy)----C(cx,cy)
		*/

		// Calculate the geometry of the walls
		const auto& rect = this->abcd;
		const auto ax = rect.GetAx();
		const auto ay = rect.GetAy();
		const auto bx = rect.GetBx();
		const auto by = rect.GetBy();
		const auto cx = rect.GetCx();
		const auto cy = rect.GetCy();
		const auto dx = rect.GetDx();
		const auto dy = rect.GetDy();

		// Top wall line geometry
		TopLine.X1 = ax;
		TopLine.Y1 = ay;
		TopLine.X2 = bx;
		TopLine.Y2 = by;

		// Right wall line geometry
		RightLine.X1 = bx;
		RightLine.Y1 = by;
		RightLine.X2 = cx;
		RightLine.Y2 = cy;

		// Bottom wall line geometry
		BottomLine.X1 = cx;
		BottomLine.Y1 = cy;
		BottomLine.X2 = dx;
		BottomLine.Y2 = dy;

		// Left wall line geometry
		LeftLine.X1 = dx;
		LeftLine.Y1 = dy;
		LeftLine.X2 = ax;
		LeftLine.Y2 = ay;

		// All walls are present by default
		walls[0] = walls[1] = walls[2] = walls[3] = IsLeftWalled = IsTopWalled = IsRightWalled = IsBottomWalled = true;
	}

	void Room::UpdateEnemyRoom(const std::shared_ptr<Enemy>& enemy)
	{
		const auto npcHotspot = enemy->TheHotspot->GetBounds();
		SDL_Rect _;

		if (SDL_IntersectRect(&InnerBounds, &npcHotspot, &_))
		{
			enemy->CurrentRoom->SetCurrentRoom(shared_from_this());
		}
	}

	void Room::Update(const unsigned long deltaMs) { /* Not need to update */ }

	ListOfEvents Room::HandleEvent(const std::shared_ptr<Event>& event, const unsigned long deltaMs)
	{
		ListOfEvents generatedEvents = {};

		if (event->Id.PrimaryId == PlayerMovedEventTypeEventId.PrimaryId)
		{
			OnPlayerMoved(generatedEvents);
		}
		else if (event->Id.PrimaryId == SettingsReloadedEventId.PrimaryId)
		{
			LoadSettings();
		}
		else if (event->Id.PrimaryId == EnemyMovedEventId.PrimaryId)
		{
			UpdateEnemyRoom(To<EnemyMovedEvent>(event)->TheEnemy);
		}
		else
		{
			std::stringstream message("Unhandled subscribed event in Room class:");
			message << event->ToString();
			Logger::Get()->LogThis(message.str());
		}
		return {};
	}

	ListOfEvents& Room::OnPlayerMoved(ListOfEvents& generatedEvents)
	{
		if (!trackEnemies) return generatedEvents;

		const auto player = To<Player>(GameData::Get()->player.lock());
		const auto playerHotSpotBounds = player->Hotspot->GetBounds();
		SDL_Rect result;

		isPlayerWithinRoom = SDL_IntersectRect(&InnerBounds, &playerHotSpotBounds, &result);
		if (isPlayerWithinRoom) { player->CurrentRoom->SetCurrentRoom(shared_from_this()); }

		return generatedEvents;
	}

	void Room::DrawWalls(SDL_Renderer* renderer) const
	{
		if (HasTopWall()) { DrawLine(renderer, TopLine); }
		if (HasRightWall()) { DrawLine(renderer, RightLine); }
		if (HasBottomWall()) { DrawLine(renderer, BottomLine); }
		if (HasLeftWall()) { DrawLine(renderer, LeftLine); }
	}

	std::shared_ptr<Room> Room::GetSideRoom(const Side side)
	{
		switch (side)
		{
		case Side::Top: return topRoom;
		case Side::Right: return rightRoom;
		case Side::Bottom: return bottomRoom;
		case Side::Left: return leftRoom;
		default: return topRoom; // should never happen  // NOLINT(clang-diagnostic-covered-switch-default)
		}
	}

	void Room::Initialize()
	{
		LoadSettings();
		SubscribeToEvent(PlayerMovedEventTypeEventId);
		SubscribeToEvent(SettingsReloadedEventId);
		SubscribeToEvent(EnemyMovedEventId);
	}

	void Room::DrawLine(SDL_Renderer* renderer, const Line& line)
	{
		SDL_RenderDrawLine(renderer, line.X1, line.Y1, line.X2, line.Y2);
	}

	void Room::DrawDiagnostics(SDL_Renderer* renderer)
	{
		constexpr SDL_Color red = { 255, 0, 0, 0 };
		constexpr SDL_Color yellow = { 255, 255, 0, 0 };
		if (fill) { DrawFilledRect(renderer, &Bounds, { 255, 0, 0, 0 }); }

		if (printDebuggingText)
		{
			const auto player = GameData::Get()->GetPlayer();

			if (printDebuggingTextNeighborsOnly)
			{
				const auto playerRoom = player->CurrentRoom->GetCurrentRoom();
				if (roomNumber == playerRoom->topRoomIndex || roomNumber == playerRoom->rightRoomIndex ||
					roomNumber == playerRoom->bottomRoomIndex || roomNumber == playerRoom->leftRoomIndex)
				{
					RectDebugging::PrintInRect(renderer, GetTag(), &Bounds, yellow);
				}

				if (roomNumber == player->CurrentRoom->TheRoom->GetRoomNumber())
				{
					RectDebugging::PrintInRect(renderer, GetTag(), &Bounds, red);
				}
			}
			else
			{
				RectDebugging::PrintInRect(renderer, GetTag(), &Bounds, yellow);
			}
		}

		if (drawHotSpot)
		{
			const SDL_Rect pointBounds = { GetPosition().GetX() - width / 2, GetPosition().GetY() + height / 2, 0, 0 };
			constexpr SDL_Color cyan = { 0, 255, 255, 0 };
			DrawFilledRect(renderer, &pointBounds, cyan);
		}

		if (drawInnerBounds)
		{
			SDL_SetRenderDrawColor(renderer, yellow.r, yellow.g, yellow.b, yellow.a);
			SDL_RenderDrawRect(renderer, &InnerBounds);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		}
	}

	void Room::Draw(SDL_Renderer* renderer)
	{
		DrawableGameObject::Draw(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Black
		DrawWalls(renderer);
		DrawDiagnostics(renderer);
	}

	void Room::LoadSettings()
	{
		GameObject::LoadSettings();

		fill = SettingsManager::Bool("room_fill", "enable");
		innerBoundsOffset = SettingsManager::Int("room", "innerBoundsOffset");
		logWallRemovals = SettingsManager::Bool("room", "logWallRemovals");
		drawInnerBounds = SettingsManager::Bool("room", "drawInnerBounds");
		drawHotSpot = SettingsManager::Bool("room", "drawHotSpot");
		printDebuggingTextNeighborsOnly = SettingsManager::Bool("global", "print_debugging_text_neighbours_only");
		printDebuggingText = SettingsManager::Bool("global", "print_debugging_text");
		trackEnemies = SettingsManager::Bool("room", "trackEnemies");

		UpdateInnerBounds();
	}

	int Room::GetX() const { return this->Position.GetX(); }
	int Room::GetY() const { return this->Position.GetY(); }

	int Room::GetWidth() const { return width; }
	int Room::GetHeight() const { return height; }

	bool Room::IsWalled(Side wall) const { return walls[static_cast<int>(wall)]; }

	bool Room::HasTopWall() const { return IsWalled(Side::Top); }
	bool Room::HasBottomWall() const { return IsWalled(Side::Bottom); }
	bool Room::HasLeftWall() const { return IsWalled(Side::Left); }
	bool Room::HasRightWall() const { return IsWalled(Side::Right); }

	AbcdRectangle& Room::GetAbcdRectangle() { return abcd; }
	Coordinate<int> Room::GetPosition() { return GetAbcdRectangle().GetCenter(); }
	int Room::GetRoomNumber() const { return roomNumber; }
	int Room::GetRowNumber(const int maxCols) const { return GetRoomNumber() / maxCols; }

	void Room::AddWall(Side wall)
	{
		this->walls[static_cast<int>(wall)] = true;
		SetWalled(wall);
	}

	void Room::RemoveWallZeroBased(Side wall)
	{
		this->walls[static_cast<int>(wall)] = false;
		SetNotWalled(wall);
	}

	void Room::ShouldRoomFill(const bool fillMe) { fill = fillMe; }

	int Room::GetColumnNumber(const int maxCols) const
	{
		const auto row = GetRowNumber(maxCols); // row for this roomNumber
		const auto rowCol0 = row * maxCols; // column 0 in this row
		const auto col = GetRoomNumber() - rowCol0; // col for this roomNumber
		return col;
	}

	std::string Room::GetName()
	{
		return "Room";
	}

	void Room::SetSurroundingRooms(const int top_index, const int rightIndex, const int bottomIndex, const int leftIndex,
		const std::vector<shared_ptr<Room>>& rooms)
	{
		this->topRoomIndex = top_index;
		this->topRoom = rooms[topRoomIndex < 1 ? 0 : topRoomIndex];
		this->rightRoomIndex = rightIndex;
		this->rightRoom = rooms[rightRoomIndex < 1 ? 0 : rightRoomIndex];
		this->bottomRoomIndex = bottomIndex;
		this->bottomRoom = rooms[bottomRoomIndex < 1 ? 0 : bottomRoomIndex];
		this->leftRoomIndex = leftIndex;
		this->leftRoom = rooms[leftRoomIndex < 1 ? 0 : leftRoomIndex];
	}

	// ReSharper disable once CppParameterNamesMismatch
	Coordinate<int> Room::GetCenter(const int w, const int h) const
	{
		const auto roomXMid = GetX() + (GetWidth() / 2);
		const auto roomYMid = GetY() + (GetHeight() / 2);
		const auto x = roomXMid - w / 2;
		const auto y = roomYMid - h / 2;
		return { x, y };
	}

	Coordinate<int> Room::GetCenter() const
	{
		return GetCenter(GetWidth(), GetHeight());
	}

	Coordinate<int> Room::GetCenter(const AbcdRectangle& rectangle) const
	{
		const auto roomXMid = GetX() + (GetWidth() / 2);
		const auto roomYMid = GetY() + (GetHeight() / 2);
		const auto x = roomXMid - rectangle.GetWidth() / 2;
		const auto y = roomYMid - rectangle.GetHeight() / 2;
		return { x, y };
	}

	int Room::GetNeighborIndex(const Side index) const
	{
		switch (index)
		{
		case Side::Top: return topRoomIndex;
		case Side::Right: return rightRoomIndex;
		case Side::Bottom: return bottomRoomIndex;
		case Side::Left: return leftRoomIndex;
		}
		return 0;
	}

	void Room::RemoveWall(Side wall)
	{
		walls[static_cast<int>(wall)] = false;
		SetNotWalled(wall);
		LogWallRemoval(wall);
	}

	void Room::LogWallRemoval(const Side wall) const
	{
		if (logWallRemovals)
		{
			std::stringstream message;
			message << "Removed " << SideUtils::SideToString(wall) << " wall in room number " << GetRoomNumber();
			Logger::Get()->LogThis(message.str());
		}
	}

	void Room::SetNotWalled(const Side wall)
	{
		switch (wall)
		{
		case Side::Top: IsTopWalled = false; break;
		case Side::Bottom: IsBottomWalled = false; break;
		case Side::Left: IsLeftWalled = false; break;
		case Side::Right: IsRightWalled = false; break;
		}
	}

	void Room::SetWalled(const Side wall)
	{
		switch (wall)
		{
		case Side::Top: { IsTopWalled = true; } break;
		case Side::Bottom: { IsBottomWalled = true; } break;
		case Side::Left: { IsLeftWalled = true; } break;
		case Side::Right: { IsRightWalled = true; } break;
		}
	}
}
