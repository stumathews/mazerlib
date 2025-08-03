#pragma once
#include <events/EventId.h>

enum EventNumber
{
	CollidedWithEnemy = 2000,
	FetchedPickup,
	Fire,
	GameWon,
	SettingsReloaded,
	GenerateNewLevel,
	InvalidMove,
	PLayerDied,
	PlayerCollidedWithPickup,
	EnemyMoved
};

const static gamelib::EventId FireEventId(Fire, "Fire");
const static gamelib::EventId FetchedPickupEventId(FetchedPickup, "FetchedPickup");
const static gamelib::EventId GameWonEventId(GameWon, "GameWon");
const static gamelib::EventId PlayerDiedEventId(PLayerDied, "PLayerDied");
const static gamelib::EventId SettingsReloadedEventId(SettingsReloaded, "SettingsReloaded");
const static gamelib::EventId InvalidMoveEventId(InvalidMove, "InvalidMove");
