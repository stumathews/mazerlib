#include "PlayerComponent.h"

PlayerComponent::PlayerComponent(const std::string& componentName, Player* player)
	: Component(componentName), ThePlayer(player), Room(0)
{
	// init members only
}
