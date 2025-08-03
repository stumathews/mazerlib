#pragma once
#include <SDL.h>

class SdlCollisionDetection
{
public:
	static bool IsColliding(const SDL_Rect* bounds1, const SDL_Rect* bounds2)
	{
		SDL_Rect result;
		return SDL_IntersectRect(bounds1, bounds2, &result);
	}
};
