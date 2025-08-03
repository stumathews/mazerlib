#pragma once
#include "GameDataManager.h"
#include "net/IElapsedTimeProvider.h"

class ElapsedGameTimeProvider : public gamelib::IElapsedTimeProvider
{
public:
	int GetElapsedTime() override
	{
		return static_cast<int>(GameDataManager::Get()->GameWorldData.ElapsedGameTime) / 1000;
	}
};
