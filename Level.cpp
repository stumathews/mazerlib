#include "Level.h"

#include <algorithm>
#include <common/constants.h>
#include <cppgamelib/events/PlayerMovedEvent.h>
#include <cppgamelib/file/SettingsManager.h>
#include <geometry/Side.h>
#include <utils/Utils.h>

#include "CharacterBuilder.h"
#include "GameDataManager.h"
#include "GameObjectMoveStrategy.h"
#include "Room.h"
#include "RoomGenerator.h"
#include "Rooms.h"
#include <tinyxml2.h>
#include "cppgamelib/character/AnimatedSprite.h"
//
#include <cppgamelib/events/EventFactory.h>
#include <cppgamelib/events/EventManager.h>

#include "pickup.h"
#include "Player.h"
#include "cppgamelib/character/DirectionUtils.h"
#include <cppgamelib/events/AddGameObjectToCurrentSceneEvent.h>

using namespace tinyxml2;
using namespace std;
using namespace gamelib;

namespace mazer
{
	Level::Level(const std::string& filename) : NumCols(0), NumRows(0), ScreenWidth(0), ScreenHeight(0), isAutoLevel(false)
	{
		FileName = filename;
		isAutoPopulatePickups = true;
	}

	Level::Level()
	{
		isAutoLevel = true;
		isAutoPopulatePickups = true;

		// Read config for level creation:
		NumRows = SettingsManager::Int("grid", "rows");
		NumCols = SettingsManager::Int("grid", "cols");
		ScreenWidth = SettingsManager::Int("global", "screen_width");
		ScreenHeight = SettingsManager::Int("global", "screen_height");
	}


	void Level::Load()
	{
		if (IsAutoLevel())
		{
			// Get Auto Level creation options:
			const auto removeRandomSidesOption = SettingsManager::Bool("grid", "removeSidesRandomly");

			// Prepare room generator
			const auto roomGenerator = RoomGenerator(ScreenWidth, ScreenHeight, NumRows, NumCols, removeRandomSidesOption);

			// Automagically self-generate the room objects for this level
			Rooms = roomGenerator.Generate();

			// We don't load the NPCs (just the rooms)
			return;
		}

		// Load the level from level file
		XMLDocument doc;

		doc.LoadFile(FileName.c_str());

		if (doc.ErrorID() == 0)
		{
			auto* scene = doc.FirstChildElement("level");

			// Read top-level level config
			NumCols = std::strtol(scene->ToElement()->Attribute("cols"), nullptr, 0);
			NumRows = std::strtol(scene->ToElement()->Attribute("rows"), nullptr, 0);
			ScreenWidth = SettingsManager::Int("global", "screen_width");
			ScreenHeight = SettingsManager::Int("global", "screen_height");

			// Check if the level file specifies that we should automatically create pickups for the level
			if (auto autoPopulatePickups = scene->ToElement()->Attribute("autoPopulatePickups"))
			{
				// Transform setting value as uppercase
				auto strToTransform = string(autoPopulatePickups);
				ranges::transform(strToTransform, strToTransform.begin(),
				                  [](unsigned char c){ return std::toupper(c); });

				isAutoPopulatePickups = strToTransform == "TRUE";
			}

			// Prase the level file's list of specified rooms
			for (auto roomNode = scene->FirstChild(); roomNode; roomNode = roomNode->NextSibling()) //  <room ...>
			{
				auto roomElement = roomNode->ToElement();
				auto number = std::strtol(roomElement->ToElement()->Attribute("number"), nullptr, 0);

				// Get visibility strings for the room sides (True/False)
				auto topSideVisible = string(roomElement->ToElement()->Attribute("top"));
				auto rightSideVisible = string(roomElement->ToElement()->Attribute("right"));
				auto bottomSideVisible = string(roomElement->ToElement()->Attribute("bottom"));
				auto leftSideVisible = string(roomElement->ToElement()->Attribute("left"));

				auto row = number / NumCols; // row for this roomNumber
				auto rowCol0 = row * NumCols;
				auto col = number - rowCol0; // col for this roomNumber

				const auto squareWidth = ScreenWidth / NumCols;
				const auto squareHeight = ScreenHeight / NumRows;
				const auto roomName = string("Room") + std::to_string(number);

				// Deserialize a room
				auto room = std::make_shared<Room>(roomName, "Room", number, col * squareWidth, row * squareHeight,
					squareWidth, squareHeight, false);

				// Local func
				auto setWall = [&](const std::string& sideVisibilityString, const Side side,
					const std::shared_ptr<Room>& inRoom) -> void
					{
						sideVisibilityString == "True" ? inRoom->AddWall(side) : inRoom->RemoveWall(side);
					};

				// Configure the room object according to its specification in the level file
				setWall(rightSideVisible, Side::Right, room);
				setWall(leftSideVisible, Side::Left, room);
				setWall(topSideVisible, Side::Top, room);
				setWall(bottomSideVisible, Side::Bottom, room);

				// Set room tag to room number
				room->SetTag(std::to_string(number));

				// Now process the objects declared inside the room element

				// <object name="xyz" type="abc" ...ie inspect all the objects in the room
				for (auto pRoomChild = roomNode->FirstChild(); pRoomChild; pRoomChild = pRoomChild->NextSibling())
				{
					string roomChildName = pRoomChild->Value();

					// Create a game object within the room
					if (roomChildName == "object") // <object ...
					{
						// Create whatever Game Object the object represents and return it as a GameObject
						auto gameObject = ParseObject(pRoomChild, room);

						// We store the player object
						if (gameObject->Type == "Player")
						{
							Player1 = To<Player>(gameObject);
						}

						// We collect pickup objects
						if (gameObject->Type == "Pickup" && !IsAutoPopulatePickups())
						{
							Pickups.push_back(To<Pickup>(gameObject));
						}

						// We collect Enemy objects
						if (gameObject->Type == "Enemy")
						{
							Enemies.push_back(To<Enemy>(gameObject));
						}
					}
				}
				Rooms.push_back(room);
			}

			// Further configure the rooms
			Rooms::ConfigureRooms(NumRows, NumCols, Rooms);

			// Configure all the objects we deserialized
			ConfigurePickups(Pickups);
			ConfigureEnemies();
		}
	}

	ListOfEvents Level::HandleEvent(const std::shared_ptr<Event>& evt, const gamelib::DeltaTime* deltaTime)
	{
		// Level itself does not handle any events
		return {};
	}

	static map<string, string> GetNodeAttributes(XMLNode* pAssetNode)
	{
		map<string, string> attributes;
		if (pAssetNode)
		{
			// Nodes need to be converted to elements to access their attributes
			for (auto attribute = pAssetNode->ToElement()->FirstAttribute(); attribute; attribute = attribute->Next())
			{
				const auto* name = attribute->Name();
				const auto* value = attribute->Value();
				attributes[name] = value;
			}
		}
		return attributes;
	}

	void Level::ConfigurePickups(const std::vector<std::shared_ptr<Pickup>>& inPickups)
	{
		for (const auto& pickup : inPickups)
		{
			pickup->LoadSettings();
			pickup->SubscribeToEvent(PlayerMovedEventTypeEventId); // pickup want to know if player moved
			GameDataManager::Get()->TheGameData()->AddPickup(pickup);

			AddGameObjectToScene(pickup);
		}
	}

	void Level::ConfigureEnemies()
	{
		for (auto& enemy : Enemies)
		{
			enemy->Initialize();
			enemy->LoadSettings();
			GameDataManager::Get()->TheGameData()->AddEnemy(enemy);

			AddGameObjectToScene(enemy);
		}
	}

	void Level::AddGameObjectToScene(const std::shared_ptr<GameObject>& object)
	{
		auto event = EventFactory::CreateAddToSceneEvent(object);
		EventManager::Get()->RaiseEvent(To<Event>(event), this);
	}


	shared_ptr<GameObject> Level::ParseObject(XMLNode* pObject, const std::shared_ptr<Room>& room) const
	{
		const auto attributes = GetNodeAttributes(pObject);
		const string objectName = attributes.at("name");
		const string objectType = attributes.at("type");
		const auto objectResourceId = stoi(attributes.at("resourceId"));

		shared_ptr<GameObject> gameObject;

		// Make Game Objects from the serialized object
		if (objectType == "Player")
		{
			gameObject = CharacterBuilder::BuildPlayer(objectName, room, objectResourceId, "playerNickName");
		}
		else if (objectType == "Pickup")
		{
			gameObject = CharacterBuilder::BuildPickup(objectName, room, objectResourceId);
		}
		else if (objectType == "Enemy")
		{
			gameObject = CharacterBuilder::BuildEnemy(objectName, room, objectResourceId, DirectionUtils::GetRandomDirection(), shared_from_this());
		}

		// Look if the game object is configured with a properties object
		for (auto pObjectChild = pObject->FirstChild(); pObjectChild; pObjectChild = pObjectChild->NextSibling())
		{
			if (pObjectChild->Value() == "property")
			{
				// Extract the property
				auto property = ParseProperty(pObjectChild, gameObject); // hmm, this object has a property attached

				auto key = std::get<0>(property);
				const auto value = std::get<1>(property);

				// Set the property as a string property
				gameObject->StringProperties[key] = value;
			}
		}
		return gameObject;
	}

	void Level::InitializePlayer(const std::shared_ptr<Player>& inPlayer, const std::shared_ptr<SpriteAsset>& spriteAsset)
	{
		inPlayer->SetMoveStrategy(std::make_shared<GameObjectMoveStrategy>(inPlayer, inPlayer->CurrentRoom));
		inPlayer->SetTag(gamelib::PlayerTag);
		inPlayer->LoadSettings();
		inPlayer->SetSprite(AnimatedSprite::Create(inPlayer->Position, spriteAsset));

		// We keep a reference to track of the player globally
		GameData::Get()->player = inPlayer;
	}

	std::tuple<std::string, std::string> Level::ParseProperty(XMLNode* pObjectChild, const shared_ptr<GameObject>& gameObject)
	{
		const auto& attributes = GetNodeAttributes(pObjectChild);
		const auto& name = attributes.at("name");
		const auto& value = attributes.at("value");

		return { name, value };
	}

	std::shared_ptr<Room> Level::GetRoom(const int row, const int col)
	{
		if (row > NumRows || col > NumCols) { return nullptr; }

		return Rooms[((row - 1) * NumCols) + col - 1];
	}
}
