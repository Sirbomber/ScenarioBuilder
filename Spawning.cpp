#include "Spawning.h"
#include <EnumSoundID.h>

void SpawnUnit(map_id type, LOCATION at, int owner, int r)
{
	Unit Unit1;
	map_id cargo = mapNone;
	if (type == mapScorpion)
	{
		cargo = mapEnergyCannon;
	}
	TethysGame::CreateUnit(Unit1, type, at, owner, cargo, r);
	if (Unit1.IsVehicle())
	{
		Unit1.DoSetLights(1);
	}
}

void SpawnConVec(map_id cargo, map_id gpWeapon, LOCATION at, int owner, int r)
{
	if (cargo != mapNone && (cargo < mapGuardPost || cargo > mapTokamak || cargo == mapMagmaWell || cargo == mapGeothermalPlant))
	{
		TethysGame::AddMessage(-1, -1, "Invalid ConVec cargo", -1, sndBld_not);
	}
	else if (cargo == mapGuardPost && (gpWeapon < mapAcidCloud || gpWeapon > mapThorsHammer || gpWeapon == mapNormalUnitExplosion))
	{
		TethysGame::AddMessage(-1, -1, "Invalid weapon for Guard Post in ConVec", -1, sndBld_not);
	}
	else
	{
		Unit Unit1;
		if (cargo == mapGuardPost)
		{
			TethysGame::CreateUnit(Unit1, mapConVec, at, owner, mapNone, r);
			Unit1.SetCargo(cargo, gpWeapon);
		}
		else
		{
			TethysGame::CreateUnit(Unit1, mapConVec, at, owner, cargo, r);
		}
		Unit1.DoSetLights(1);

	}
}

void SpawnTruck(Truck_Cargo cargo, int amt, LOCATION at, int owner, int r)
{
	Unit Unit1;
	TethysGame::CreateUnit(Unit1, mapCargoTruck, at, owner, mapNone, r);
	if (amt == 0)
	{
		if (cargo == truckGarbage)
		{
			amt = 11999;
		}
		else if (cargo == truckSpaceport)
		{
			amt = (int)mapEDWARDSatellite;
		}
		else
		{
			amt = 1000;
		}
	}
	if (cargo != truckEmpty)
	{
		Unit1.SetTruckCargo(cargo, amt);
	}
	Unit1.DoSetLights(1);
}

void SpawnCombatUnit(map_id type, map_id weapon, LOCATION at, int owner, int r)
{
	if (weapon < mapAcidCloud || weapon > mapThorsHammer || weapon == mapNormalUnitExplosion)
	{
		TethysGame::AddMessage(-1, -1, "Invalid weapon", -1, sndBld_not);
	}
	else
	{
		Unit Unit1;
		TethysGame::CreateUnit(Unit1, type, at, owner, weapon, r);
		if (type != mapGuardPost)
		{
			Unit1.DoSetLights(1);
		}
	}
}

void InternalSpawn(int type, int cargo, int amt, int x, int y, int spawnFor, int r, int l)
{
	Unit Unit1;
	TethysGame::CreateUnit(Unit1, (map_id)type, LOCATION(x, y), spawnFor, mapNone, r);
	if (type == mapConVec && cargo == mapGuardPost)
	{
		Unit1.SetCargo(mapGuardPost, mapLaser);	// temp
	}
	else if (type != mapCargoTruck)
	{
		Unit1.SetCargo((map_id)cargo, mapNone);
	}
	else
	{
		Unit1.SetCargo(mapNone, mapNone);
		Unit1.SetTruckCargo((Truck_Cargo)cargo, amt);
	}
	if (Unit1.IsVehicle() && l != 0)
	{
		Unit1.DoSetLights(1);
	}
}
