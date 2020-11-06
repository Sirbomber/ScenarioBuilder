#pragma once

#include <Outpost2DLL.h>

void SpawnUnit(map_id type, LOCATION at, int owner, int r = 0);
void SpawnConVec(map_id cargo, map_id gpWeapon, LOCATION at, int owner, int r = 0);
void SpawnTruck(Truck_Cargo cargo, int amt, LOCATION at, int owner, int r = 0);
void SpawnCombatUnit(map_id type, map_id weapon, LOCATION at, int owner, int r = 0);

void InternalSpawn(int type, int cargo, int amt, int x, int y, int spawnFor, int r, int l);