#pragma once
#include "GameDataManager.h"
#include "cppgamelib/net/IElapsedTimeProvider.h"

namespace mazer
{
	class ElapsedGameTimeProvider : public gamelib::IElapsedTimeProvider
	{
	public:
		int GetElapsedTime() override
		{
			return static_cast<int>(GameDataManager::Get()->GameWorldData.ElapsedGameTime) / 1000;
		}
	};
}
