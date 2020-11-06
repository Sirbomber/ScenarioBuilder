#include <Outpost2DLL.h>
#include <OP2Helper.h>
#include <HFL.h>
#include <Patches.h>

#include <OP2Types.h>
#include <Outpost2App.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <time.h>

#include "ExportMap.h"

// Output streams
std::ifstream input;
std::ofstream output;
std::stringstream commonList, rareList, fumaroleList, magmaList, wreckageList, tubeList, wallList, playerUnitList[7];

// ----------------------------------------------------------------------------------------------------------------------------------------
// File output/save driver
void Save()
{
	time_t rawtime;
	struct tm * timeinfo;
	char filename[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(filename, 80, "unitlist-%F-%H-%M-%S.txt", timeinfo);

	output.open(filename);

	if (!output.is_open())
	{
		TethysGame::AddMessage(-1, -1, "Failed to open file for writing", -1, sndDirt);
		return;
	}

	GenerateUnitLists(script);
	GenerateTubesAndWallsLists(script);

	if (!commonList.str().empty())
	{
		output << commonList.rdbuf();
	}
	if (!rareList.str().empty())
	{
		output << rareList.rdbuf();
	}
	if (!fumaroleList.str().empty())
	{
		output << fumaroleList.rdbuf();
	}
	if (!magmaList.str().empty())
	{
		output << magmaList.rdbuf();
	}
	for (int i = 0; i < 7; i++)
	{
		if (!playerUnitList[i].str().empty())
		{
			output << playerUnitList[i].rdbuf();
		}
	}
	if (!tubeList.str().empty())
	{
		output << tubeList.rdbuf();
	}
	if (!wallList.str().empty())
	{
		output << wallList.rdbuf();
	}

	output.close();
	TethysGame::AddMessage(-1, -1, "Command list saved!", -1, sndMessage2);
}

void GenerateCpp()
{
	time_t rawtime;
	struct tm * timeinfo;
	char filename[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(filename, 80, "cpp-%F-%H-%M-%S.txt", timeinfo);

	//output.open("unit_output.txt");
	output.open(filename);

	if (!output.is_open())
	{
		TethysGame::AddMessage(-1, -1, "Failed to open file for writing", -1, sndDirt);
		return;
	}

	GenerateUnitLists();
	GenerateTubesAndWallsLists();

	output << "UnitEx Unit1;\n\n";

	if (!commonList.str().empty())
	{
		output << "// Common Ore Beacons\n";
		output << commonList.rdbuf();
		output << "\n";
	}
	if (!rareList.str().empty())
	{
		output << "// Rare Ore Beacons\n";
		output << rareList.rdbuf();
		output << "\n";
	}
	if (!fumaroleList.str().empty())
	{
		output << "// Fumaroles\n";
		output << fumaroleList.rdbuf();
		output << "\n";
	}
	if (!magmaList.str().empty())
	{
		output << "// Magma Vents\n";
		output << magmaList.rdbuf();
		output << "\n";
	}
	for (int i = 0; i < 7; i++)
	{
		if (!playerUnitList[i].str().empty())
		{
			output << "// Player " << i + 1 << " Units\n";
			output << playerUnitList[i].rdbuf();
			output << "\n";
		}
	}
	if (!tubeList.str().empty())
	{
		output << "// Tubes\n";
		output << tubeList.rdbuf();
		output << "\n";
	}
	if (!wallList.str().empty())
	{
		output << "// Walls\n";
		output << wallList.rdbuf();
		output << "\n";
	}

	output.close();
	TethysGame::AddMessage(-1, -1, "C++ code generated successfully!", -1, sndMessage2);
}

void GeneratePython()
{
	//
}

// ----------------------------------------------------------------------------------------------------------------------------------------
// Unit placement code generation

void GenerateUnitLists(outputType saveAs)
{
	UnitEx Unit1;
	OP2Unit *internalUnit;

	internalUnit = &(*unitArray)[0];
	while (internalUnit->unitIndex != internalUnit->next->unitIndex && internalUnit->next->unitIndex != 1024)
	{
		internalUnit = internalUnit->next;
		Unit1.unitID = internalUnit->unitIndex;

		map_id type = Unit1.GetType();
		if (type == mapMiningBeacon)
		{
			if ((int)internalUnit->Beacon.type == 0)
			{
				switch (saveAs)
				{
				case cpp:
					commonList << "TethysGame::CreateBeacon(" << MapIdToString(type) << ", " << Unit1.Location().x << ", " << Unit1.Location().y << ", " << (int)internalUnit->Beacon.type << ", " << internalUnit->Beacon.yield << ", " << internalUnit->Beacon.variant << ");\n";
					break;
				case python:
					break;
				case script:
					commonList << "/common " << Unit1.Location().x << " " << Unit1.Location().y << " " << internalUnit->Beacon.yield << " " << internalUnit->Beacon.variant << " 1\n";
					break;
				}
			}
			else
			{
				switch (saveAs)
				{
				case cpp:
					rareList << "TethysGame::CreateBeacon(" << MapIdToString(type) << ", " << Unit1.Location().x << ", " << Unit1.Location().y << ", " << (int)internalUnit->Beacon.type << ", " << internalUnit->Beacon.yield << ", " << internalUnit->Beacon.variant << ");\n";
					break;
				case python:
					break;
				case script:
					rareList << "/rare " << Unit1.Location().x << " " << Unit1.Location().y << " " << internalUnit->Beacon.yield << " " << internalUnit->Beacon.variant << " 1\n";
					break;
				}				
			}
		}
		else if (type == mapFumarole)
		{
			switch (saveAs)
			{
			case cpp:
				fumaroleList << "TethysGame::CreateBeacon(" << MapIdToString(type) << ", " << Unit1.Location().x << ", " << Unit1.Location().y << ", -1, -1, -1);\n";
				break;
			case python:
				break;
			case script:
				fumaroleList << "/fumarole " << Unit1.Location().x << " " << Unit1.Location().y << " 1\n";
				break;
			}
		}
		else if (type == mapMagmaVent)
		{
			switch (saveAs)
			{
			case cpp:
				magmaList << "TethysGame::CreateBeacon(" << MapIdToString(type) << ", " << Unit1.Location().x << ", " << Unit1.Location().y << ", -1, -1, -1);\n";
				break;
			case python:
				break;
			case script:
				magmaList << "/magmavent " << Unit1.Location().x << " " << Unit1.Location().y << " 1\n";
				break;
			}
		}
		else if (type == mapWreckage)
		{
			switch (saveAs)
			{
			case cpp:
				wreckageList << "TethysGame::CreateWreck(" << Unit1.Location().x << ", " << Unit1.Location().y << ", (map_id)" << Unit1.GetCargo() << ", false);\n";
				break;
			case python:
				break;
			case script:
				// Not implemented
				break;
			}
			
		}
		else if (type == mapCargoTruck)
		{
			switch (saveAs)
			{
			case cpp:
				playerUnitList[Unit1.OwnerID()] << "TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(" << Unit1.Location().x << ", " << Unit1.Location().y << "), " << Unit1.OwnerID() << ", mapNone, " << NormalizeRotation(internalUnit->rotation) << ");\n";
				if (Unit1.GetLights())
				{
					playerUnitList[Unit1.OwnerID()] << "Unit1.DoSetLights(1);\n";
				}
				if (Unit1.GetCargoType() != truckEmpty)
				{
					playerUnitList[Unit1.OwnerID()] << "Unit1.SetTruckCargo(" << TruckCargoToString(Unit1.GetCargoType()) << ", " << Unit1.GetCargoAmount() << ");\n";
				}
				break;
			case python:
				break;
			case script:
				playerUnitList[Unit1.OwnerID()] << "/is " << Unit1.GetType() << " " << Unit1.GetCargoType() << " " << Unit1.GetCargoAmount() << " " << Unit1.OwnerID() << " " << Unit1.Location().x << " " << Unit1.Location().y << " " << NormalizeRotation(internalUnit->rotation) << " " << Unit1.GetLights() << "\n";
			}
		}
		else if (type == mapStructureFactory || type == mapSpaceport)
		{
			switch (saveAs)
			{
			case cpp:
				playerUnitList[Unit1.OwnerID()] << "TethysGame::CreateUnit(Unit1, " << MapIdToString(type) << ", LOCATION(" << Unit1.Location().x << ", " << Unit1.Location().y << "), " << Unit1.OwnerID() << ", mapNone, " << NormalizeRotation(internalUnit->rotation) << ");\n";
				for (int i = 0; i < 6; i++)
				{
					if (Unit1.GetFactoryCargo(i) != mapNone)
					{
						playerUnitList[Unit1.OwnerID()] << "Unit1.SetFactoryCargo(" << i << ", " << MapIdToString(Unit1.GetFactoryCargo(i)) << ", " << MapIdToString(Unit1.GetFactoryCargoWeapon(i)) << ");\n";
					}
				}
				break;
			case python:
				break;
			case script:
				playerUnitList[Unit1.OwnerID()] << "/is " << Unit1.GetType() << " " << "0" << " " << "0" << " " << Unit1.OwnerID() << " " << Unit1.Location().x << " " << Unit1.Location().y << " " << NormalizeRotation(internalUnit->rotation) << " " << Unit1.GetLights() << "\n";
				break;
			}
		}
		// Don't save disasters
		else if (type >= mapLightning && type <= mapMeteor)
		{
			//
		}
		else
		{
			switch (saveAs)
			{
			case cpp:
				if (type == mapConVec && Unit1.GetCargo() == mapGuardPost)
				{
					playerUnitList[Unit1.OwnerID()] << "TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(" << Unit1.Location().x << ", " << Unit1.Location().y << "), " << Unit1.OwnerID() << ", mapNone, " << NormalizeRotation(internalUnit->rotation) << ");\n";
					playerUnitList[Unit1.OwnerID()] << "Unit1.SetCargo(mapGuardPost, mapLaser);\n";	// TEMP: mapLaser -- Figure out how to get ConVec Guard Post kit weapon
				}
				else
				{
					playerUnitList[Unit1.OwnerID()] << "TethysGame::CreateUnit(Unit1, " << MapIdToString(type) << ", LOCATION(" << Unit1.Location().x << ", " << Unit1.Location().y << "), " << Unit1.OwnerID() << ", " << MapIdToString(Unit1.GetCargo()) << ", " << NormalizeRotation(internalUnit->rotation) << ");\n";
				}
				if (Unit1.IsVehicle() && Unit1.GetLights())
				{
					playerUnitList[Unit1.OwnerID()] << "Unit1.DoSetLights(1);\n";
				}
				break;
			case python:
				break;
			case script:
				playerUnitList[Unit1.OwnerID()] << "/is " << ((Unit1.GetType() == mapRareOreMine || Unit1.GetType() == mapMagmaWell) ? mapCommonOreMine : Unit1.GetType()) << " " << Unit1.GetCargo() << " " << ((Unit1.GetType() == mapConVec && Unit1.GetCargo() == mapGuardPost) ? Unit1.GetCargoAmount() : 0) << " " << Unit1.OwnerID() << " " << Unit1.Location().x << " " << Unit1.Location().y << " " << NormalizeRotation(internalUnit->rotation) << " " << Unit1.GetLights() << "\n";
				break;
			}
		}
	}
}

void GenerateTubesAndWallsLists(outputType saveAs)
{
	//output << "// Walls\n";
	LOCATION mapSize = LOCATION(1024, 1024);
	mapSize.Clip();
	if (mapSize.x == 0)
	{
		mapSize.x = 512;
	}
	mapSize.y += 1;
	for (int x = 0; x < mapSize.x; x++)
	{
		for (int y = 0; y < mapSize.y; y++)
		{
			int tile = GameMap::GetTile(LOCATION(x, y));

			// Tubes
			if ((tile >= 417 && tile <= 432) ||
				(tile >= 929 && tile <= 944) ||
				(tile >= 1678 && tile <= 1693))
			{
				switch (saveAs)
				{
				case cpp:
					tubeList << "TethysGame::CreateWallOrTube(" << x << ", " << y << ", -1, mapTube);\n";
					break;
				case python:
					break;
				case script:
					tubeList << "/itw " << mapTube << " " << x << " " << y << "\n";
					break;
				}
			}

			// Walls
			else if (GameMap::GetCellType(LOCATION(x, y)) == cellNormalWall)
			{
				switch (saveAs)
				{
				case cpp:
					wallList << "TethysGame::CreateWallOrTube(" << x << ", " << y << ", -1, mapWall);\n";
					break;
				case python:
					break;
				case script:
					tubeList << "/itw " << mapWall << " " << x << " " << y << "\n";
					break;
				}
			}

			else if (GameMap::GetCellType(LOCATION(x, y)) == cellMicrobeWall)
			{
				switch (saveAs)
				{
				case cpp:
					wallList << "TethysGame::CreateWallOrTube(" << x << ", " << y << ", -1, mapMicrobeWall);\n";
					break;
				case python:
					break;
				case script:
					tubeList << "/itw " << mapMicrobeWall << " " << x << " " << y << "\n";
					break;
				}
			}

			else if (GameMap::GetCellType(LOCATION(x, y)) == cellLavaWall)
			{
				switch (saveAs)
				{
				case cpp:
					wallList << "TethysGame::CreateWallOrTube(" << x << ", " << y << ", -1, mapLavaWall);\n";
					break;
				case python:
					break;
				case script:
					tubeList << "/itw " << mapLavaWall << " " << x << " " << y << "\n";
					break;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------------------------------------------------------------------
// AI code generation
void GenerateAiUnitGroupSetup()
{
	//
}

// ----------------------------------------------------------------------------------------------------------------------------------------
// Helper functions

std::string MapIdToString(map_id type)
{
	switch (type)
	{
		// Generic
	case mapAny:
		return "mapAny";
		break;
	case mapNone:
	case 127:				// Not sure why this happens
		return "mapNone";
		break;

		// Vehicles
	case mapCargoTruck:
		return "mapCargoTruck";
		break;
	case mapConVec:
		return "mapConVec";
		break;
	case mapSpider:
		return "mapSpider";
		break;
	case mapScorpion:
		return "mapScorpion";
		break;
	case mapLynx:
		return "mapLynx";
		break;
	case mapPanther:
		return "mapPanther";
		break;
	case mapTiger:
		return "mapTiger";
		break;
	case mapRoboSurveyor:
		return "mapRoboSurveyor";
		break;
	case mapRoboMiner:
		return "mapRoboMiner";
		break;
	case mapGeoCon:
		return "mapGeoCon";
		break;
	case mapScout:
		return "mapScout";
		break;
	case mapRoboDozer:
		return "mapRoboDozer";
		break;
	case mapEvacuationTransport:
		return "mapEvacuationTransport";
		break;
	case mapRepairVehicle:
		return "mapRepairVehicle";
		break;
	case mapEarthworker:
		return "mapEarthworker";
		break;
	case mapSmallCapacityAirTransport:
		return "mapSmallCapacityAirTransport";
		break;

		// Earthworks
	case mapTube:
		return "mapTube";
		break;
	case mapWall:
		return "mapWall";
		break;
	case mapLavaWall:
		return "mapLavaWall";
		break;
	case mapMicrobeWall:
		return "mapMicrobeWall";
		break;

		// Structures
	case mapCommonOreMine:
		return "mapCommonOreMine";
		break;
	case mapRareOreMine:
		return "mapCommonOreMine";		// This is intentional; directly spawning rare ore mines is buggy.
		break;
	case mapGuardPost:
		return "mapGuardPost";
		break;
	case mapLightTower:
		return "mapLightTower";
		break;
	case mapCommonStorage:
		return "mapCommonStorage";
		break;
	case mapRareStorage:
		return "mapRareStorage";
		break;
	case mapForum:
		return "mapForum";
		break;
	case mapCommandCenter:
		return "mapCommandCenter";
		break;
	case mapMHDGenerator:
		return "mapMHDGenerator";
		break;
	case mapResidence:
		return "mapResidence";
		break;
	case mapRobotCommand:
		return "mapRobotCommand";
		break;
	case mapTradeCenter:
		return "mapTradeCenter";
		break;
	case mapBasicLab:
		return "mapBasicLab";
		break;
	case mapMedicalCenter:
		return "mapMedicalCenter";
		break;
	case mapNursery:
		return "mapNursery";
		break;
	case mapSolarPowerArray:
		return "mapSolarPowerArray";
		break;
	case mapRecreationFacility:
		return "mapRecreationFacility";
		break;
	case mapUniversity:
		return "mapUniversity";
		break;
	case mapAgridome:
		return "mapAgridome";
		break;
	case mapDIRT:
		return "mapDIRT";
		break;
	case mapGarage:
		return "mapGarage";
		break;
	case mapMagmaWell:
		return "mapCommonOreMine";		// This is intentional; directly spawning magma wells on top of magma vents is buggy.
		break;
	case mapMeteorDefense:
		return "mapMeteorDefense";
		break;
	case mapGeothermalPlant:
		return "mapGeothermalPlant";
		break;
	case mapArachnidFactory:
		return "mapArachnidFactory";
		break;
	case mapConsumerFactory:
		return "mapConsumerFactory";
		break;
	case mapStructureFactory:
		return "mapStructureFactory";
		break;
	case mapVehicleFactory:
		return "mapVehicleFactory";
		break;
	case mapStandardLab:
		return "mapStandardLab";
		break;
	case mapAdvancedLab:
		return "mapAdvancedLab";
		break;
	case mapObservatory:
		return "mapObservatory";
		break;
	case mapReinforcedResidence:
		return "mapReinforcedResidence";
		break;
	case mapAdvancedResidence:
		return "mapAdvancedResidence";
		break;
	case mapCommonOreSmelter:
		return "mapCommonOreSmelter";
		break;
	case mapSpaceport:
		return "mapSpaceport";
		break;
	case mapRareOreSmelter:
		return "mapRareOreSmelter";
		break;
	case mapGORF:
		return "mapGORF";
		break;
	case mapTokamak:
		return "mapTokamak";
		break;

		// Weapons
	case mapAcidCloud:
		return "mapAcidCloud";
		break;
	case mapEMP:
		return "mapEMP";
		break;
	case mapLaser:
		return "mapLaser";
		break;
	case mapMicrowave:
		return "mapMicrowave";
		break;
	case mapRailGun:
		return "mapRailGun";
		break;
	case mapRPG:
		return "mapRPG";
		break;
	case mapStarflare:					// Unit version
	case mapStarflare2:					// GP version
		return "mapStarflare";
		break;
	case mapSupernova:					// Unit version
	case mapSupernova2:					// GP version
		return "mapSupernova";
		break;
	case mapNormalUnitExplosion:		// Self-destruct blast
		return "mapNormalUnitExplosion";
		break;
	case mapESG:
		return "mapESG";
		break;
	case mapStickyfoam:
		return "mapStickyfoam";
		break;
	case mapThorsHammer:
		return "mapThorsHammer";
		break;
	case mapEnergyCannon:
		return "mapEnergyCannon";
		break;
	case mapBlast:
		return "mapBlast";
		break;
	case 0x4B:							// Unused
		return "mapNone";
		break;

		// Disasters
	case mapLightning:
		return "mapLightning";
		break;
	case mapVortex:
		return "mapVortex";
		break;
	case mapEarthquake:
		return "mapEarthquake";
		break;
	case mapEruption:
		return "mapEruption";
		break;
	case mapMeteor:
		return "mapMeteor";
		break;

		// Beacons
	case mapMiningBeacon:
		return "mapMiningBeacon";
		break;
	case mapMagmaVent:
		return "mapMagmaVent";
		break;
	case mapFumarole:
		return "mapFumarole";
		break;
	case mapWreckage:
		return "mapWreckage";
		break;

		// Explosions
	case mapDisasterousBuildingExplosion:
		return "mapDisasterousBuildingExplosion";
		break;
	case mapCatastrophicBuildingExplosion:
		return "mapCatastrophicBuildingExplosion";
		break;
	case mapAtheistBuildingExplosion:
		return "mapAtheistBuildingExplosion";
		break;

		// SPEHSS
	case mapEDWARDSatellite:
		return "mapEDWARDSatellite";
		break;
	case mapSolarSatellite:
		return "mapSolarSatellite";
		break;
	case mapIonDriveModule:
		return "mapIonDriveModule";
		break;
	case mapFusionDriveModule:
		return "mapFusionDriveModule";
		break;
	case mapCommandModule:
		return "mapCommandModule";
		break;
	case mapFuelingSystems:
		return "mapFuelingSystems";
		break;
	case mapHabitatRing:
		return "mapHabitatRing";
		break;
	case mapSensorPackage:
		return "mapSensorPackage";
		break;
	case mapSkydock:
		return "mapSkydock";
		break;
	case mapStasisSystems:
		return "mapStasisSystems";
		break;
	case mapOrbitalPackage:
		return "mapOrbitalPackage";
		break;
	case mapPhoenixModule:
		return "mapPhoenixModule";
		break;
	case mapRareMetalsCargo:
		return "mapRareMetalsCargo";
		break;
	case mapCommonMetalsCargo:
		return "mapCommonMetalsCargo";
		break;
	case mapFoodCargo:
		return "mapFoodCargo";
		break;
	case mapEvacuationModule:
		return "mapEvacuationModule";
		break;
	case mapChildrenModule:
		return "mapChildrenModule";
		break;

		// Rockets
	case mapSULV:
		return "mapSULV";
		break;
	case mapRLV:
		return "mapRLV";
		break;
	case mapEMPMissile:
		return "mapEMPMissile";
		break;

		// Etc (don't use)
	case mapImpulseItems:
		return "mapImpulseItems";
		break;
	case mapWares:
		return "mapWares";
		break;
	case mapLuxuryWares:
		return "mapLuxuryWares";
		break;
	case mapInterColonyShuttle:
		return "mapInterColonyShuttle";
		break;
	case mapSpider3Pack:
		return "mapSpider3Pack";
		break;
	case mapScorpion3Pack:
		return "mapScorpion3Pack";
		break;
	case mapPrettyArt:
		return "mapPrettyArt";
		break;
	case mapGeneralUnit:
		return "mapGeneralUnit";
		break;
	default:
		return std::to_string(type);
		break;
	}
}

std::string TruckCargoToString(Truck_Cargo cargo)
{
	switch (cargo)
	{
	case truckEmpty:
		return "truckEmpty";
		break;
	case truckFood:
		return "truckFood";
		break;
	case truckCommonOre:
		return "truckCommonOre";
		break;
	case truckRareOre:
		return "truckRareOre";
		break;
	case truckCommonMetal:
		return "truckCommonMetal";
		break;
	case truckRareMetal:
		return "truckRareMetal";
		break;
	case truckCommonRubble:
		return "truckCommonRubble";
		break;
	case truckRareRubble:
		return "truckRareRubble";
		break;
	case truckSpaceport:
		return "truckSpaceport";
		break;
	case truckGarbage:					// Wreckage
		return "truckGarbage";
		break;
	case truckUnit:						// I have no idea what this is
		return "truckUnit";
		break;
	default:
		return std::to_string(cargo);
		break;
	}
}

int NormalizeRotation(char r)
{
	// Internally, rotation is stored from -128 to 96, but when creating units, we use values from 0 (east) - 7 (northeast)
	// -128 == 4 (west)
	//  -96 == 5 (northwest)
	//  -64 == 6 (north)
	//  -32 == 7 (northeast)
	//    0 == 0 (east)
	//   32 == 1 (southeast)
	//   64 == 2 (south)
	//   96 == 3 (southwest)
	int iR = (int)r;
	if (iR < -96)
	{
		return 4;
	}
	else if (iR < -64)
	{
		return 5;
	}
	else if (iR < -32)
	{
		return 6;
	}
	else if (iR < 0)
	{
		return 7;
	}
	else if (iR < 32)
	{
		return 0;
	}
	else if (iR < 64)
	{
		return 1;
	}
	else if (iR < 96)
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

std::string MapIdToChatId(map_id type)
{
	// Chat string IDs are just the map ID strings without the leading "map".  So just call MapIdToString and remove the first 3 characters.
	std::string chatId = MapIdToString(type);
	chatId.erase(0, 3);
	return chatId;
}

std::string TruckCargoToChatId(Truck_Cargo cargo)
{
	// Chat string IDs are just the map ID strings without the leading "truck".  So just call TruckCargoToString and remove the first 5 characters.
	std::string chatId = TruckCargoToString(cargo);
	chatId.erase(0, 5);
	return chatId;
}

map_id ChatIdToMapId(const char* type)
{
	// This only checks for "valid" units and structures.  Technical "units" (mapNone, mapPrettyArt), starship parts, and beacons will all return mapNone (0).
	if (strcmp(type, "CargoTruck") == 0)
	{
		return mapCargoTruck;
	}
	else if (strcmp(type, "ConVec") == 0)
	{
		return mapConVec;
	}
	else if (strcmp(type, "Spider") == 0)
	{
		return mapSpider;
	}
	else if (strcmp(type, "Scorpion") == 0)
	{
		return mapScorpion;
	}
	else if (strcmp(type, "Lynx") == 0)
	{
		return mapLynx;
	}
	else if (strcmp(type, "Panther") == 0)
	{
		return mapPanther;
	}
	else if (strcmp(type, "Tiger") == 0)
	{
		return mapTiger;
	}
	else if (strcmp(type, "RoboSurveyor") == 0)
	{
		return mapRoboSurveyor;
	}
	else if (strcmp(type, "RoboMiner") == 0)
	{
		return mapRoboMiner;
	}
	else if (strcmp(type, "GeoCon") == 0)
	{
		return mapGeoCon;
	}
	else if (strcmp(type, "Scout") == 0)
	{
		return mapScout;
	}
	else if (strcmp(type, "RoboDozer") == 0)
	{
		return mapRoboDozer;
	}
	else if (strcmp(type, "EvacuationTransport") == 0)
	{
		return mapEvacuationTransport;
	}
	else if (strcmp(type, "RepairVehicle") == 0)
	{
		return mapRepairVehicle;
	}
	else if (strcmp(type, "Earthworker") == 0)
	{
		return mapEarthworker;
	}
	else if (strcmp(type, "CommonOreMine") == 0)
	{
		return mapCommonOreMine;
	}
	else if (strcmp(type, "RareOreMine") == 0)
	{
		return mapCommonOreMine;
	}
	else if (strcmp(type, "GuardPost") == 0)
	{
		return mapGuardPost;
	}
	else if (strcmp(type, "LightTower") == 0)
	{
		return mapLightTower;
	}
	else if (strcmp(type, "CommonStorage") == 0)
	{
		return mapCommonStorage;
	}
	else if (strcmp(type, "RareStorage") == 0)
	{
		return mapRareStorage;
	}
	else if (strcmp(type, "Forum") == 0)
	{
		return mapForum;
	}
	else if (strcmp(type, "CommandCenter") == 0)
	{
		return mapCommandCenter;
	}
	else if (strcmp(type, "MHDGenerator") == 0)
	{
		return mapMHDGenerator;
	}
	else if (strcmp(type, "Residence") == 0)
	{
		return mapResidence;
	}
	else if (strcmp(type, "RobotCommand") == 0)
	{
		return mapRobotCommand;
	}
	else if (strcmp(type, "TradeCenter") == 0)
	{
		return mapTradeCenter;
	}
	else if (strcmp(type, "BasicLab") == 0)
	{
		return mapBasicLab;
	}
	else if (strcmp(type, "MedicalCenter") == 0)
	{
		return mapMedicalCenter;
	}
	else if (strcmp(type, "Nursery") == 0)
	{
		return mapNursery;
	}
	else if (strcmp(type, "SolarPowerArray") == 0)
	{
		return mapSolarPowerArray;
	}
	else if (strcmp(type, "RecreationFacility") == 0)
	{
		return mapRecreationFacility;
	}
	else if (strcmp(type, "University") == 0)
	{
		return mapUniversity;
	}
	else if (strcmp(type, "Agridome") == 0)
	{
		return mapAgridome;
	}
	else if (strcmp(type, "DIRT") == 0)
	{
		return mapDIRT;
	}
	else if (strcmp(type, "Garage") == 0)
	{
		return mapGarage;
	}
	else if (strcmp(type, "MagmaWell") == 0)
	{
		return mapCommonOreMine;
	}
	else if (strcmp(type, "MeteorDefense") == 0)
	{
		return mapMeteorDefense;
	}
	else if (strcmp(type, "GeothermalPlant") == 0)
	{
		return mapGeothermalPlant;
	}
	else if (strcmp(type, "ArachnidFactory") == 0)
	{
		return mapArachnidFactory;
	}
	else if (strcmp(type, "ConsumerFactory") == 0)
	{
		return mapConsumerFactory;
	}
	else if (strcmp(type, "StructureFactory") == 0)
	{
		return mapStructureFactory;
	}
	else if (strcmp(type, "VehicleFactory") == 0)
	{
		return mapVehicleFactory;
	}
	else if (strcmp(type, "StandardLab") == 0)
	{
		return mapStandardLab;
	}
	else if (strcmp(type, "AdvancedLab") == 0)
	{
		return mapAdvancedLab;
	}
	else if (strcmp(type, "Observatory") == 0)
	{
		return mapObservatory;
	}
	else if (strcmp(type, "ReinforcedResidence") == 0)
	{
		return mapReinforcedResidence;
	}
	else if (strcmp(type, "AdvancedResidence") == 0)
	{
		return mapAdvancedResidence;
	}
	else if (strcmp(type, "CommonOreSmelter") == 0)
	{
		return mapCommonOreSmelter;
	}
	else if (strcmp(type, "Spaceport") == 0)
	{
		return mapSpaceport;
	}
	else if (strcmp(type, "RareOreSmelter") == 0)
	{
		return mapRareOreSmelter;
	}
	else if (strcmp(type, "GORF") == 0)
	{
		return mapGORF;
	}
	else if (strcmp(type, "Tokamak") == 0)
	{
		return mapTokamak;
	}
	else if (strcmp(type, "AcidCloud") == 0)
	{
		return mapAcidCloud;
	}
	else if (strcmp(type, "EMP") == 0)
	{
		return mapEMP;
	}
	else if (strcmp(type, "Laser") == 0)
	{
		return mapLaser;
	}
	else if (strcmp(type, "Microwave") == 0)
	{
		return mapMicrowave;
	}
	else if (strcmp(type, "RailGun") == 0)
	{
		return mapRailGun;
	}
	else if (strcmp(type, "RPG") == 0)
	{
		return mapRPG;
	}
	else if (strcmp(type, "Starflare") == 0)
	{
		return mapStarflare;
	}
	else if (strcmp(type, "Supernova") == 0)
	{
		return mapSupernova;
	}
	else if (strcmp(type, "ESG") == 0)
	{
		return mapESG;
	}
	else if (strcmp(type, "Stickyfoam") == 0)
	{
		return mapStickyfoam;
	}
	else if (strcmp(type, "ThorsHammer") == 0)
	{
		return mapThorsHammer;
	}
	else
	{
		return mapNone;
	}
}

Truck_Cargo ChatIdToTruckCargo(const char* type)
{
	if (strcmp(type, "Food") == 0)
	{
		return truckFood;
	}
	else if (strcmp(type, "CommonOre") == 0)
	{
		return truckCommonOre;
	}
	else if (strcmp(type, "RareOre") == 0)
	{
		return truckRareOre;
	}
	else if (strcmp(type, "CommonMetal") == 0)
	{
		return truckCommonMetal;
	}
	else if (strcmp(type, "RareMetal") == 0)
	{
		return truckRareMetal;
	}
	else if (strcmp(type, "CommonRuble") == 0)
	{
		return truckCommonRubble;
	}
	else if (strcmp(type, "RareRubble") == 0)
	{
		return truckRareRubble;
	}
	else if (strcmp(type, "Spaceport") == 0)
	{
		return truckSpaceport;
	}
	else if (strcmp(type, "Garbage") == 0)
	{
		return truckGarbage;
	}
	else
	{
		return truckEmpty;
	}
}

// ----------------------------------------------------------------------------------------------------------------------------------------
// Load script

struct cmdChat
{
	char sourcePlayerId;
	unsigned char toPlayerIds; // (bit flags, a 1 means that player gets the message)
	char msgBuff[0x64];
};

std::stringstream commandList;
Trigger ProcessLoad;

void LoadScript(const char *filename)
{
	input.open(filename);

	if (!input.is_open())
	{
		TethysGame::AddMessage(-1, -1, "Failed to open script file for reading", -1, sndDirt);
		return;
	}

	CommandPacket packet;
	packet.type = 0x30;
	cmdChat chat;
	chat.sourcePlayerId = TethysGame::LocalPlayer();
	//chat.toPlayerIds = 63;	// Visible to all players (for testing)
	chat.toPlayerIds = 0;		// Invisible (even to the sending player)

	// Replay chat commands across the network
	std::string nextLine;
	while (std::getline(input, nextLine))
	{
#ifdef _SOLO
		// Build the chat command packet
		std::strcpy(chat.msgBuff, nextLine.c_str());
		packet.dataLength = 3 + nextLine.length();
		packet.timeStamp = ExtPlayer[TethysGame::LocalPlayer()].GetNextCommandPacketAddress()->timeStamp;
		packet.unknown = ExtPlayer[TethysGame::LocalPlayer()].GetNextCommandPacketAddress()->unknown;
		memcpy(packet.dataBuff, &chat, sizeof(cmdChat));

		// Process it
		ExtPlayer[TethysGame::LocalPlayer()].ProcessCommandPacket(&packet);
#else
		commandList << nextLine << "\n";
#endif
	}

#ifndef _SOLO
	ProcessLoad = CreateTimeTrigger(1, 0, 1, "ProcessSavedCommandsList");
#endif

	input.close();
}

Export void ProcessSavedCommandsList()
{
	CommandPacket *nextPacket;
	nextPacket = ExtPlayer[TethysGame::LocalPlayer()].GetNextCommandPacketAddress();
	if (nextPacket->type == ctNop || nextPacket->type == ctInvalidCommand)
	{
		cmdChat chat;
		chat.sourcePlayerId = TethysGame::LocalPlayer();
		//chat.toPlayerIds = 63;	// Visible to all players (for testing)
		chat.toPlayerIds = 0;		// Invisible (even to the sending player)

		std::string nextLine;
		std::getline(commandList, nextLine);
		std::strcpy(chat.msgBuff, nextLine.c_str());
		nextPacket->type = 0x30;
		nextPacket->dataLength = 3 + nextLine.length();
		memcpy(nextPacket->dataBuff, &chat, sizeof(cmdChat));

		if (commandList.eof())
		{
			ProcessLoad.Destroy();
			commandList.clear();
		}
	}
}
