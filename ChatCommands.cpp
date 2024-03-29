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

#include "ChatCommands.h"
#include "ExportMap.h"
#include "Spawning.h"

struct AiVfData
{
	BuildingGroup vf;
	std::list<ScGroup> reinforcedGroups;
};

struct AiData
{
	std::list<MiningGroup> allMiningGroups;
	std::list<BuildingGroup> allBuildingGroups;
	std::list<FightGroup> allFightGroups;
	std::list<AiVfData> allVFs;
};
AiData ai[7];

// Max length of chat string
//const int msgSize = 36 + 1;
const int msgSize = 83 + 1;
int confirmClearMark = -1;
bool commandsDisabled = false;

// Chat commands used for debugging purposes.
void ProcessChatCommands(char *chatText, int sourcePlayerNum)
{
	if (!chatText || strcmp(chatText, "/") == 0 || chatText[0] != '/')
		return;

	// Tokenize chat string
	int i = 0;
	char buf[msgSize];
	strcpy_s(buf, sizeof(buf), chatText);
	char word[10][msgSize], *ptok = NULL, *pch = strtok_s(buf, " /", &ptok);
	do
	{

		strcpy_s(word[i], sizeof(word[0]), pch);
		++i;
	} while ((pch = strtok_s(NULL, " ", &ptok)) != 0 && i < (sizeof(word) / sizeof(word[0])));

	// Needed for checks when spawning units
	LOCATION mapSize = LOCATION(1024, 1024);
	int xOffset;
	mapSize.Clip();

	// Get actual map dimensions
	if (mapSize.x == 0)
	{
		mapSize.x = 512;
		xOffset = -1;
	}
	else
	{
		mapSize.x -= 32;
		xOffset = +31;
	}
	mapSize.y += 1;

	// ---------------------------------------------------------------------------------------------------------
	// Output commands - these commands generate code/a script file on the local player's client only and can be called at any time during the game.
	if (strcmp(word[0], "save") == 0)
	{
		if (sourcePlayerNum != TethysGame::LocalPlayer())
		{
			return;
		}
		else
		{
			Save();
		}
	}

	else if (strcmp(word[0], "cpp") == 0)
	{
		if (sourcePlayerNum != TethysGame::LocalPlayer())
		{
			return;
		}
		else
		{
			GenerateCpp();
		}
	}

	else if (strcmp(word[0], "cpp_legacy") == 0)
	{
		if (sourcePlayerNum != TethysGame::LocalPlayer())
		{
			return;
		}
		else
		{
			GenerateLegacyCpp();
		}
	}

	if (strcmp(word[0], "python") == 0)
	{
		if (sourcePlayerNum != TethysGame::LocalPlayer())
		{
			return;
		}
		else
		{
			GeneratePython();
		}
	}

	// ---------------------------------------------------------------------------------------------------------
	// All other commands impact the state of the game and cannot be called once the /startLOS command has been used to prevent cheating.
	else if (commandsDisabled)
	{
		return;
	}

	// ---------------------------------------------------------------------------------------------------------
	// Editor commands - these change the state of the game, map, units, or players
	else if (strcmp(word[0], "setres") == 0)
	{
		int newValue = 0;
		char* c;
		newValue = strtol(word[2], &c, 0);
		if (strcmp(word[1], "common") == 0)
		{
			Player[sourcePlayerNum].SetOre(newValue);
		}
		else if (strcmp(word[1], "rare") == 0)
		{
			Player[sourcePlayerNum].SetRareOre(newValue);
		}
		else if (strcmp(word[1], "food") == 0)
		{
			Player[sourcePlayerNum].SetFoodStored(newValue);
		}
		else if (strcmp(word[1], "kids") == 0)
		{
			Player[sourcePlayerNum].SetKids(newValue);
		}
		else if (strcmp(word[1], "workers") == 0)
		{
			Player[sourcePlayerNum].SetWorkers(newValue);
		}
		else if (strcmp(word[1], "scientists") == 0)
		{
			Player[sourcePlayerNum].SetScientists(newValue);
		}
	}

	else if (strcmp(word[0], "setTechLevel") == 0)
	{
		int techLevel = 0;
		char* c;
		techLevel = strtol(word[1], &c, 0);
		if (techLevel < 0)
		{
			TethysGame::AddMessage(-1, -1, "Tech level cannot be negative.", -1, sndBld_not);
		}
		else
		{
			Player[sourcePlayerNum].SetTechLevel(techLevel);
		}
	}

	else if (strcmp(word[0], "spawn") == 0)
	{
		LOCATION mapSize = LOCATION(1024, 1024);
		int xOffset;
		mapSize.Clip();

		// Get actual map dimensions
		if (mapSize.x == 0)
		{
			mapSize.x = 512;
			xOffset = -1;
		}
		else
		{
			mapSize.x -= 32;
			xOffset = +31;
		}
		mapSize.y += 1;

		int x, y, spawnFor;
		char* c;
		x = strtol(word[3], &c, 0);
		y = strtol(word[4], &c, 0);
		spawnFor = strtol(word[2], &c, 0);

		if (x < 1 || y < 1 || x > mapSize.x || y > mapSize.y)
		{
			TethysGame::AddMessage(-1, -1, "Spawn location invalid", -1, sndBld_not);
			return;
		}

		if (spawnFor < 0 || spawnFor > 6)
		{
			TethysGame::AddMessage(-1, -1, "Invalid player", -1, sndBld_not);
			return;
		}

		LOCATION at = LOCATION(x + xOffset, y - 1);
		map_id ToSpawn = ChatIdToMapId(word[1]);
		if (ToSpawn == mapNone)
		{
			TethysGame::AddMessage(-1, -1, "Unknown unit type", -1, sndBld_not);
		}
		else if (ToSpawn == mapConVec)
		{
			SpawnConVec(ChatIdToMapId(word[5]), ChatIdToMapId(word[6]), at, spawnFor);
		}
		else if (ToSpawn == mapCargoTruck)
		{
			SpawnTruck(ChatIdToTruckCargo(word[5]), strtol(word[6], &c, 0), at, spawnFor);
		}
		else if (ToSpawn == mapLynx || ToSpawn == mapPanther || ToSpawn == mapTiger || ToSpawn == mapGuardPost)
		{
			SpawnCombatUnit(ToSpawn, ChatIdToMapId(word[5]), at, spawnFor);
		}
		else
		{
			SpawnUnit(ToSpawn, at, spawnFor);
		}
	}

	else if (strcmp(word[0], "dc") == 0)
	{
		if (!TethysGame::CanHaveDisasters())
		{
			TethysGame::AddMessage(-1, -1, "Disasters have not been enabled.", -1, sndBld_not);
			return;
		}

		char *c;
		int disChance = strtol(word[2], &c, 0);

		if (strcmp(word[1], "quake") == 0)
		{
			DC.EnableDisaster(disQuake);
			DC.SetDisasterTypeWeight(disQuake, disChance);
		}
		else if (strcmp(word[1], "storm") == 0)
		{
			DC.EnableDisaster(disStorm);
			DC.SetDisasterTypeWeight(disStorm, disChance);
		}
		else if (strcmp(word[1], "vortex") == 0)
		{
			DC.EnableDisaster(disVortex);
			DC.SetDisasterTypeWeight(disVortex, disChance);
		}
		else if (strcmp(word[1], "meteor") == 0)
		{
			DC.EnableDisaster(disMeteor);
			DC.SetDisasterTypeWeight(disMeteor, disChance);
		}
	}
	else if (strcmp(word[0], "setplayer") == 0)
	{
		char *c;
		int newPlayerId = strtol(word[1], &c, 0);
		if (Player[newPlayerId].IsHuman())
		{
			TethysGame::AddMessage(-1, -1, "Can't take over a human player.", -1, sndBld_not);
		}
		else if (newPlayerId >= 0 && newPlayerId <= 6)
		{
			Player[sourcePlayerNum].GoAI();
			if (sourcePlayerNum == TethysGame::LocalPlayer())
			{
				OP2Game *op2GameObj = (OP2Game*)gameObj;
				op2GameObj->localPlayer = newPlayerId;
			}
			Player[newPlayerId].GoHuman();
		}
	}

	else if (strcmp(word[0], "eden") == 0)
	{
		Player[sourcePlayerNum].GoEden();
	}

	else if (strcmp(word[0], "plymouth") == 0)
	{
		Player[sourcePlayerNum].GoPlymouth();
	}

	else if (strcmp(word[0], "common") == 0)
	{
		int x, y, yield = 0, variant = 0, noOffset = 0;
		char *c;
		x = strtol(word[1], &c, 0);
		y = strtol(word[2], &c, 0);
		yield = strtol(word[3], &c, 0);
		variant = strtol(word[4], &c, 0);
		noOffset = strtol(word[5], &c, 0);		// Intended for saving/loading only
		if (noOffset != 0)
		{
			TethysGame::CreateBeacon(mapMiningBeacon, x, y, 0, yield, variant);
		}
		else if (x > 1 && x < mapSize.x && y > 1 && y < mapSize.y)
		{
			TethysGame::CreateBeacon(mapMiningBeacon, x + xOffset, y - 1, 0, yield, variant);
		}
	}

	else if (strcmp(word[0], "rare") == 0)
	{
		int x, y, yield = 0, variant = 0, noOffset = 0;
		char *c;
		x = strtol(word[1], &c, 0);
		y = strtol(word[2], &c, 0);
		yield = strtol(word[3], &c, 0);
		variant = strtol(word[4], &c, 0);
		noOffset = strtol(word[5], &c, 0);		// Intended for saving/loading only
		if (noOffset != 0)
		{
			TethysGame::CreateBeacon(mapMiningBeacon, x, y, 1, yield, variant);
		}
		else if (x > 1 && x < mapSize.x && y > 1 && y < mapSize.y)
		{
			TethysGame::CreateBeacon(mapMiningBeacon, x + xOffset, y - 1, 1, yield, variant);
		}
	}

	else if (strcmp(word[0], "fumarole") == 0)
	{
		int x, y, noOffset = 0;
		char *c;
		x = strtol(word[1], &c, 0);
		y = strtol(word[2], &c, 0);
		noOffset = strtol(word[3], &c, 0);		// Intended for saving/loading only
		if (noOffset != 0)
		{
			TethysGame::CreateBeacon(mapFumarole, x, y, -1, -1, -1);
		}
		else if (x > 1 && x < mapSize.x && y > 1 && y < mapSize.y)
		{
			TethysGame::CreateBeacon(mapFumarole, x + xOffset, y - 1, -1, -1, -1);
		}
	}

	else if (strcmp(word[0], "magmavent") == 0)
	{
		int x, y, noOffset = 0;
		char *c;
		x = strtol(word[1], &c, 0);
		y = strtol(word[2], &c, 0);
		noOffset = strtol(word[3], &c, 0);		// Intended for saving/loading only
		if (noOffset != 0)
		{
			TethysGame::CreateBeacon(mapMagmaVent, x, y, -1, -1, -1);
		}
		else if (x > 1 && x < mapSize.x && y > 1 && y < mapSize.y)
		{
			TethysGame::CreateBeacon(mapMagmaVent, x + xOffset, y - 1, -1, -1, -1);
		}
	}

	else if (strcmp(word[0], "wreck") == 0)
	{
		int x, y, tech = 11999, visible = 0, noOffset = 0;
		char* c;
		x = strtol(word[1], &c, 0);
		y = strtol(word[2], &c, 0);
		tech = strtol(word[3], &c, 0);
		visible = strtol(word[4], &c, 0);
		noOffset = strtol(word[5], &c, 0);		// Intended for saving/loading only
		if (noOffset != 0)
		{
			TethysGame::CreateWreck(x, y, (map_id)tech, visible);
		}
		else if (x > 1 && x < mapSize.x && y > 1 && y < mapSize.y)
		{
			TethysGame::CreateWreck(x + xOffset, y - 1, (map_id)tech, visible);
		}
	}

	// ---------------------------------------------------------------------------------------------------------
	// Host-only Commands - these control the editor/game session
	else if (strcmp(word[0], "load") == 0)
	{
		if (sourcePlayerNum != 0)
		{
			TethysGame::AddMessage(-1, -1, "Only the host may load a script", -1, sndBld_not);
			return;
		}
		else if (TethysGame::LocalPlayer() == sourcePlayerNum)
		{
			LoadScript(word[1]);
		}
	}

	else if (strcmp(word[0], "startLOS") == 0)
	{
		if (sourcePlayerNum != 0)
		{
			TethysGame::AddMessage(-1, -1, "Only the host may enable LOS mode", -1, sndBld_not);
			return;
		}

		commandsDisabled = true;

		char* c;
		int keepRes = 0;
		keepRes = strtol(word[1], &c, 0);

		// Set default player resources so everyone gets a fair start!
		if (keepRes == 0)
		{
			for (int i = 0; i < TethysGame::NoPlayers(); i++)
			{
				switch (Player[i].Difficulty())
				{
				case 0:
					Player[i].SetKids(22);
					Player[i].SetWorkers(27);
					Player[i].SetScientists(18);
					Player[i].SetOre(6000);
					Player[i].SetFoodStored(3000);
					Player[i].MarkResearchComplete(3303);	// Health Maintenance
					Player[i].MarkResearchComplete(3304);	// Offspring Enhancement
					Player[i].MarkResearchComplete(3305);	// Research Training Programs
					Player[i].MarkResearchComplete(3401);	// Cybernetic Teleoperation
					break;
				case 1:
					Player[i].SetKids(20);
					Player[i].SetWorkers(24);
					Player[i].SetScientists(15);
					Player[i].SetOre(4000);
					Player[i].SetFoodStored(2000);
					Player[i].MarkResearchComplete(3303);	// Health Maintenance
					Player[i].MarkResearchComplete(3304);	// Offspring Enhancement
					Player[i].MarkResearchComplete(3305);	// Research Training Programs
					break;
				case 2:
					Player[i].SetKids(18);
					Player[i].SetWorkers(20);
					Player[i].SetScientists(12);
					Player[i].SetOre(2000);
					Player[i].SetFoodStored(1000);
					Player[i].MarkResearchComplete(3303);	// Health Maintenance
					break;
				}
				ExtPlayer[i].SetSatelliteCount(mapEDWARDSatellite, 0);
			}
		}

		// Re-point rare ore tech correctly to hide beacons
		for (int i = 0; i < ((OP2Research*)researchObj)->numTechs; ++i)
		{
			if (((OP2Research*)researchObj)->techInfos[i]->techID == 5110)
			{
				unitInfoArray[mapRareOreMine]->techId = i;
				break;
			}
		}

		// Create standard LOS victory conditions
		CreateLastOneStandingVictoryCondition();
		TethysGame::AddMessage(-1, -1, "New objective.", -1, sndSavant30);

		// Enable disasters
		if (TethysGame::CanHaveDisasters())
		{
			//DC.SetMapSize(mapSize.x, mapSize.y);
			DC.SetCallbackTrigger("DisasterCreator_Callback", 200);
		}

		// Enable morale
		if (TethysGame::UsesMorale())
		{
			TethysGame::FreeMoraleLevel(-1);
		}
	}

	else if (strcmp(word[0], "clear") == 0)
	{
		if (sourcePlayerNum != 0)
		{
			TethysGame::AddMessage(-1, -1, "Only the host may clear all units", -1, sndBld_not);
		}
		else if (confirmClearMark == -1 || TethysGame::Tick() > (confirmClearMark + 500))
		{
			confirmClearMark = TethysGame::Tick() + 500;
			TethysGame::AddMessage(-1, -1, "Erase all units?  Enter /clear again to confirm.", -1, sndLight);
		}
		else
		{
			confirmClearMark = -1;
			UnitEx Unit1;
			OP2Unit *internalUnit;

			// Clear units and buildings
			internalUnit = &(*unitArray)[0];
			while (internalUnit->unitIndex != internalUnit->next->unitIndex && internalUnit->next->unitIndex != 1024)
			{
				internalUnit = internalUnit->next;
				Unit1.unitID = internalUnit->unitIndex;
				if ((Unit1.GetType() >= mapCargoTruck && Unit1.GetType() <= mapSmallCapacityAirTransport) ||
					(Unit1.GetType() >= mapCommonOreMine && Unit1.GetType() <= mapTokamak) ||
					(Unit1.GetType() >= mapMiningBeacon && Unit1.GetType() <= mapWreckage))
				{
					Unit1.DoPoof();
				}
			}

			TethysGame::AddMessage(-1, -1, "All units erased.", -1, sndLight);
		}
	}

	else if (strcmp(word[0], "changemap") == 0)
	{
		if (sourcePlayerNum != 0)
		{
			TethysGame::AddMessage(-1, -1, "Only the host may change the map", -1, sndBld_not);
		}
		else
		{
			if (((OP2Map*)mapObj)->numUnits > 0)
			{
				TethysGame::AddMessage(-1, -1, "Can't change map after units have been placed.", -1, sndBld_not);
			}
			else
			{
				GameMapEx::LoadMap(word[1]);
				for (int i = 0; i < 7; i++)
				{
					Player[i].CenterViewOn(0, 0);
				}
			}
		}
	}

	// ---------------------------------------------------------------------------------------------------------
	// AI Commands - use these to set up rudimentary AI opponents (WIP)
	else if (strcmp(word[0], "ai") == 0)
	{
		int aiNo = -1, x1 = -1, y1 = -1, x2 = -1, y2 = -1;
		char *c;
		aiNo = strtol(word[2], &c, 0);
		x1 = strtol(word[3], &c, 0);
		y1 = strtol(word[4], &c, 0);
		x2 = strtol(word[5], &c, 0);
		y2 = strtol(word[6], &c, 0);

		if (aiNo < 0 || aiNo > 6)
		{
			TethysGame::AddMessage(-1, -1, "Invalid parameters.", -1, sndBld_not);
		}
		else if (Player[aiNo].IsHuman())
		{
			TethysGame::AddMessage(-1, -1, "AI functions cannot be applied to human players.", -1, sndBld_not);
		}

		else if (strcmp(word[1], "mining") == 0)
		{
			// Find all smelters, mines, and beacons in the passed-in area
			UnitEx Unit1;
			OP2Unit *internalUnit;
			std::list<UnitEx> rareSmelters;
			std::list<UnitEx> rareMines;
			std::list<UnitEx> rareBeacons;
			std::list<UnitEx> commonSmelters;
			std::list<UnitEx> commonMines;
			std::list<UnitEx> commonBeacons;


			InRectEnumerator FindMiningStuff(MAP_RECT(x1 + xOffset, y1 - 1, x2 + xOffset, y2));
			while (FindMiningStuff.GetNext(Unit1))
			{
				if (Unit1.OwnerID() == 6)
				{
					internalUnit = &(*unitArray)[Unit1.unitID];
					if (Unit1.GetType() == mapMagmaVent || internalUnit->Beacon.type == 1)
					{
						rareBeacons.push_back(Unit1);
					}
					else if (internalUnit->Beacon.type == 0)
					{
						commonBeacons.push_back(Unit1);
					}
				}
				else if (Unit1.OwnerID() == aiNo)
				{
					switch (Unit1.GetType())
					{
					case mapCommonOreSmelter:
						commonSmelters.push_back(Unit1);
						break;
					case mapRareOreSmelter:
						rareSmelters.push_back(Unit1);
						break;
					case mapCommonOreMine:
						commonMines.push_back(Unit1);
						break;
					case mapRareOreMine:
					case mapMagmaWell:
						rareMines.push_back(Unit1);
						break;
					}
				}
			}

			// Check if the area is too big
			if (commonMines.size() > 1 || rareMines.size() > 1)
			{
				TethysGame::AddMessage(-1, -1, "Too many mines found!  Try a smaller area.", -1, sndBld_not);
				return;
			}

			// No mines in area
			//else if (commonBeacons.size() == 0 && rareBeacons.size() == 0)
			if (commonMines.size() == 0 && rareMines.size() == 0)
			{
				TethysGame::AddMessage(-1, -1, "No mines found!", -1, sndBld_not);
				return;
			}

			// Match up common mines to common smelters
			if (commonMines.size() == 1 && commonSmelters.size() != 0)
			{
				for (std::list<UnitEx>::iterator i = commonSmelters.begin(); i != commonSmelters.end(); i++)
				{
					MiningGroup temp = CreateMiningGroup(aiNo);
					temp.SetTargCount(mapCargoTruck, mapNone, 2);

					// Try to find idle trucks near this smelter
					unsigned long unused;
					int numTrucksFound = 0;
					ClosestEnumerator findTrucks(LOCATION(i->Location()));
					while (findTrucks.GetNext(Unit1, unused) && numTrucksFound < 2)
					{

						internalUnit = &(*unitArray)[Unit1.unitID];
						//char string[100];
						//scr_snprintf(string, 100, "Unit ID: %d at (%d, %d) -- Group ID: %d\n", Unit1.unitID, Unit1.Location().x, Unit1.Location().y, (int)internalUnit->scGroupIndex);
						//TethysGame::AddMessage(-1, -1, string, -1, sndMessage2);
						if (Unit1.GetType() == mapCargoTruck && Unit1.OwnerID() == aiNo && internalUnit->scGroupIndex == 255)
						{
							temp.TakeUnit(Unit1);
							numTrucksFound++;
						}
					}
					temp.Setup(commonMines.front(), *i, MAP_RECT(commonMines.front().Location(), i->Location()));
					ai[aiNo].allMiningGroups.push_back(temp);

					if (!ai[aiNo].allVFs.empty())
					{
						AiVfData *tempVf = NULL;
						for (std::list<AiVfData>::iterator j = ai[aiNo].allVFs.begin(); j != ai[aiNo].allVFs.end(); j++)
						{
							if (tempVf == NULL || j->reinforcedGroups.size() < tempVf->reinforcedGroups.size())
							{
								tempVf = &(*j);
							}
						}
						tempVf->reinforcedGroups.push_back(temp);
						tempVf->vf.RecordVehReinforceGroup(temp, 1000);
					}
				}
			}

			// Match up rare mines to rare smelters
			if (rareMines.size() == 1 && rareSmelters.size() != 0)
			{
				for (std::list<UnitEx>::iterator i = rareSmelters.begin(); i != rareSmelters.end(); i++)
				{
					MiningGroup temp = CreateMiningGroup(aiNo);
					temp.SetTargCount(mapCargoTruck, mapNone, 2);

					// Try to find idle trucks near this smelter
					unsigned long unused;
					int numTrucksFound = 0;
					ClosestEnumerator findTrucks(LOCATION(i->Location()));
					while (findTrucks.GetNext(Unit1, unused) && numTrucksFound < 2)
					{

						internalUnit = &(*unitArray)[Unit1.unitID];
						//char string[100];
						//scr_snprintf(string, 100, "Unit ID: %d at (%d, %d) -- Group ID: %d\n", Unit1.unitID, Unit1.Location().x, Unit1.Location().y, (int)internalUnit->scGroupIndex);
						//TethysGame::AddMessage(-1, -1, string, -1, sndMessage2);
						if (Unit1.GetType() == mapCargoTruck && Unit1.OwnerID() == aiNo && internalUnit->scGroupIndex == 255)
						{
							temp.TakeUnit(Unit1);
							numTrucksFound++;
						}
					}
					temp.Setup(rareMines.front(), *i, MAP_RECT(rareMines.front().Location(), i->Location()));
					ai[aiNo].allMiningGroups.push_back(temp);

					if (!ai[aiNo].allVFs.empty())
					{
						AiVfData *tempVf = NULL;
						for (std::list<AiVfData>::iterator j = ai[aiNo].allVFs.begin(); j != ai[aiNo].allVFs.end(); j++)
						{
							if (tempVf == NULL || j->reinforcedGroups.size() < tempVf->reinforcedGroups.size())
							{
								tempVf = &(*j);
							}
						}
						tempVf->reinforcedGroups.push_back(temp);
						tempVf->vf.RecordVehReinforceGroup(temp, 1000);
					}
				}
			}
		}

		else if (strcmp(word[1], "building") == 0)
		{
			//
		}

		else if (strcmp(word[1], "reinforce") == 0)
		{
			int aiNo = -1;
			aiNo = strtol(word[2], &c, 0);

			if (aiNo < 0 || aiNo > 6)
			{
				TethysGame::AddMessage(-1, -1, "Invalid parameters.", -1, sndBld_not);
			}

			// Clear any existing VF data
			std::list<AiVfData>::iterator i = ai[aiNo].allVFs.begin();
			while (i != ai[aiNo].allVFs.end())
			{
				i->vf.Destroy();
				ai[aiNo].allVFs.erase(i++);
			}

			// Populate the VF list.
			UnitEx Unit1;
			PlayerBuildingEnum findVFs(aiNo, mapVehicleFactory);
			while (findVFs.GetNext(Unit1))
			{
				// This check should be pointless.  Should be.
				if (Unit1.IsLive() && Unit1.GetType() == mapVehicleFactory && Unit1.OwnerID() == aiNo)
				{
					AiVfData tempVf;
					tempVf.vf = CreateBuildingGroup(aiNo);
					tempVf.vf.TakeUnit(Unit1);
					ai[aiNo].allVFs.push_back(tempVf);
				}
			}

			// Now split the burden of reinforcing groups evenly.
			if (!ai[aiNo].allVFs.empty())
			{
				std::list<AiVfData>::iterator curVF = ai[aiNo].allVFs.begin();

				// Building groups
				for (std::list<BuildingGroup>::iterator curBG = ai[aiNo].allBuildingGroups.begin(); curBG != ai[aiNo].allBuildingGroups.end(); curBG++)
				{
					curVF->reinforcedGroups.push_back(*curBG);
					curVF->vf.RecordVehReinforceGroup(*curBG, 1000);
					curVF++;
					if (curVF == ai[aiNo].allVFs.end())
					{
						curVF = ai[aiNo].allVFs.begin();
					}
				}

				// Mining groups
				for (std::list<MiningGroup>::iterator curMG = ai[aiNo].allMiningGroups.begin(); curMG != ai[aiNo].allMiningGroups.end(); curMG++)
				{
					curVF->reinforcedGroups.push_back(*curMG);
					curVF->vf.RecordVehReinforceGroup(*curMG, 1000);
					curVF++;
					if (curVF == ai[aiNo].allVFs.end())
					{
						curVF = ai[aiNo].allVFs.begin();
					}
				}

				// Fight groups
				for (std::list<FightGroup>::iterator curFG = ai[aiNo].allFightGroups.begin(); curFG != ai[aiNo].allFightGroups.end(); curFG++)
				{
					curVF->reinforcedGroups.push_back(*curFG);
					curVF->vf.RecordVehReinforceGroup(*curFG, 1000);
					curVF++;
					if (curVF == ai[aiNo].allVFs.end())
					{
						curVF = ai[aiNo].allVFs.begin();
					}
				}
			}
		}

		// Defenders
		else if (strcmp(word[1], "defenders") == 0)
		{
			UnitEx Unit1;
			InRectEnumerator FindUnits(MAP_RECT(x1 + xOffset, y1 - 1, x2 + xOffset, y2));
			int unitsFound = 0,
				lynx_Laser = 0,
				lynx_Microwave = 0,
				lynx_RailGun = 0,
				lynx_RPG = 0,
				lynx_EMP = 0,
				lynx_AcidCloud = 0,
				lynx_ESG = 0,
				lynx_Foam = 0,
				lynx_Thors = 0,
				lynx_Flare = 0,
				lynx_Nova = 0,
				//
				pnth_Laser = 0,
				pnth_Microwave = 0,
				pnth_RailGun = 0,
				pnth_RPG = 0,
				pnth_EMP = 0,
				pnth_AcidCloud = 0,
				pnth_ESG = 0,
				pnth_Foam = 0,
				pnth_Thors = 0,
				pnth_Flare = 0,
				pnth_Nova = 0,
				//
				tgr_Laser = 0,
				tgr_Microwave = 0,
				tgr_RailGun = 0,
				tgr_RPG = 0,
				tgr_EMP = 0,
				tgr_AcidCloud = 0,
				tgr_ESG = 0,
				tgr_Foam = 0,
				tgr_Thors = 0,
				tgr_Flare = 0,
				tgr_Nova = 0,
				//
				scorpions = 0;

			FightGroup temp = CreateFightGroup(aiNo);

			while (FindUnits.GetNext(Unit1) && unitsFound < 32)
			{
				if (Unit1.GetType() == mapLynx || Unit1.GetType() == mapPanther || Unit1.GetType() == mapTiger || Unit1.GetType() == mapScorpion)
				{
					temp.TakeUnit(Unit1);
					unitsFound++;
				}

				switch (Unit1.GetType())
				{
				case mapScorpion:
					scorpions++;
					break;
				case mapLynx:
					switch (Unit1.GetCargo())
					{
					case mapLaser:
						lynx_Laser++;
						break;
					case mapMicrowave:
						lynx_Microwave++;
						break;
					case mapRailGun:
						lynx_RailGun++;
						break;
					case mapRPG:
						lynx_RPG++;
						break;
					case mapEMP:
						lynx_EMP++;
						break;
					case mapAcidCloud:
						lynx_AcidCloud++;
						break;
					case mapESG:
						lynx_ESG++;
						break;
					case mapStickyfoam:
						lynx_Foam++;
						break;
					case mapThorsHammer:
						lynx_Thors++;
						break;
					case mapStarflare:
						lynx_Flare++;
						break;
					case mapSupernova:
						lynx_Nova++;
						break;
					}
					break;
				case mapPanther:
					switch (Unit1.GetCargo())
					{
					case mapLaser:
						pnth_Laser++;
						break;
					case mapMicrowave:
						pnth_Microwave++;
						break;
					case mapRailGun:
						pnth_RailGun++;
						break;
					case mapRPG:
						pnth_RPG++;
						break;
					case mapEMP:
						pnth_EMP++;
						break;
					case mapAcidCloud:
						pnth_AcidCloud++;
						break;
					case mapESG:
						pnth_ESG++;
						break;
					case mapStickyfoam:
						pnth_Foam++;
						break;
					case mapThorsHammer:
						pnth_Thors++;
						break;
					case mapStarflare:
						pnth_Flare++;
						break;
					case mapSupernova:
						pnth_Nova++;
						break;
					}
					break;
				case mapTiger:
					switch (Unit1.GetCargo())
					{
					case mapLaser:
						tgr_Laser++;
						break;
					case mapMicrowave:
						tgr_Microwave++;
						break;
					case mapRailGun:
						tgr_RailGun++;
						break;
					case mapRPG:
						tgr_RPG++;
						break;
					case mapEMP:
						tgr_EMP++;
						break;
					case mapAcidCloud:
						tgr_AcidCloud++;
						break;
					case mapESG:
						tgr_ESG++;
						break;
					case mapStickyfoam:
						tgr_Foam++;
						break;
					case mapThorsHammer:
						tgr_Thors++;
						break;
					case mapStarflare:
						tgr_Flare++;
						break;
					case mapSupernova:
						tgr_Nova++;
						break;
					}
					break;
				}
			}

			if (unitsFound == 0)
			{
				temp.Destroy();
			}
			else
			{
				temp.SetRect(MAP_RECT(x1 + xOffset, y1 - 1, x2 + xOffset, y2));

				temp.SetTargCount(mapScorpion, mapEnergyCannon, scorpions);
				temp.SetTargCount(mapLynx, mapLaser, lynx_Laser);
				temp.SetTargCount(mapLynx, mapMicrowave, lynx_Microwave);
				temp.SetTargCount(mapLynx, mapRailGun, lynx_RailGun);
				temp.SetTargCount(mapLynx, mapRPG, lynx_RPG);
				temp.SetTargCount(mapLynx, mapEMP, lynx_EMP);
				temp.SetTargCount(mapLynx, mapAcidCloud, lynx_AcidCloud);
				temp.SetTargCount(mapLynx, mapESG, lynx_ESG);
				temp.SetTargCount(mapLynx, mapStickyfoam, lynx_Foam);
				temp.SetTargCount(mapLynx, mapThorsHammer, lynx_Thors);
				temp.SetTargCount(mapLynx, mapStarflare, lynx_Flare);
				temp.SetTargCount(mapLynx, mapSupernova, lynx_Nova);

				temp.SetTargCount(mapPanther, mapLaser, pnth_Laser);
				temp.SetTargCount(mapPanther, mapMicrowave, pnth_Microwave);
				temp.SetTargCount(mapPanther, mapRailGun, pnth_RailGun);
				temp.SetTargCount(mapPanther, mapRPG, pnth_RPG);
				temp.SetTargCount(mapPanther, mapEMP, pnth_EMP);
				temp.SetTargCount(mapPanther, mapAcidCloud, pnth_AcidCloud);
				temp.SetTargCount(mapPanther, mapESG, pnth_ESG);
				temp.SetTargCount(mapPanther, mapStickyfoam, pnth_Foam);
				temp.SetTargCount(mapPanther, mapThorsHammer, pnth_Thors);
				temp.SetTargCount(mapPanther, mapStarflare, pnth_Flare);
				temp.SetTargCount(mapPanther, mapSupernova, pnth_Nova);

				temp.SetTargCount(mapTiger, mapLaser, tgr_Laser);
				temp.SetTargCount(mapTiger, mapMicrowave, tgr_Microwave);
				temp.SetTargCount(mapTiger, mapRailGun, tgr_RailGun);
				temp.SetTargCount(mapTiger, mapRPG, tgr_RPG);
				temp.SetTargCount(mapTiger, mapEMP, tgr_EMP);
				temp.SetTargCount(mapTiger, mapAcidCloud, tgr_AcidCloud);
				temp.SetTargCount(mapTiger, mapESG, tgr_ESG);
				temp.SetTargCount(mapTiger, mapStickyfoam, tgr_Foam);
				temp.SetTargCount(mapTiger, mapThorsHammer, tgr_Thors);
				temp.SetTargCount(mapTiger, mapStarflare, tgr_Flare);
				temp.SetTargCount(mapTiger, mapSupernova, tgr_Nova);

				ai[aiNo].allFightGroups.push_back(temp);

				if (!ai[aiNo].allVFs.empty())
				{
					AiVfData *tempVf = NULL;
					for (std::list<AiVfData>::iterator j = ai[aiNo].allVFs.begin(); j != ai[aiNo].allVFs.end(); j++)
					{
						if (tempVf == NULL || j->reinforcedGroups.size() < tempVf->reinforcedGroups.size())
						{
							tempVf = &(*j);
						}
					}
					tempVf->reinforcedGroups.push_back(temp);
					tempVf->vf.RecordVehReinforceGroup(temp, 1000);
				}
			}
		}

		// Offense
		else if (strcmp(word[1], "difficulty") == 0)
		{
			int difficulty = -1;
			difficulty = strtol(word[3], &c, 0);
			x1 = strtol(word[4], &c, 0);
			y1 = strtol(word[5], &c, 0);
			x2 = strtol(word[6], &c, 0);
			y2 = strtol(word[7], &c, 0);
		}
	}

	// ---------------------------------------------------------------------------------------------------------
	// Internal Commands - technically usable by players, but likely to crash since there's no error/range checking of arguments

	// Internal spawning command, intended only to be used by the /load logic.  No sanity checking of input occurs.
	// Format: /is (Unit) (Cargo) (CargoAmt) (Owner) (X) (Y) (Rotation) (LightsOn)
	else if (strcmp(word[0], "is") == 0)
	{
		int type, cargo, amt, x, y, spawnFor, r, l;
		char *c;
		type = strtol(word[1], &c, 0);
		cargo = strtol(word[2], &c, 0);
		amt = strtol(word[3], &c, 0);
		spawnFor = strtol(word[4], &c, 0);
		x = strtol(word[5], &c, 0);
		y = strtol(word[6], &c, 0);
		r = strtol(word[7], &c, 0);
		l = strtol(word[8], &c, 0);
		
		InternalSpawn(type, cargo, amt, x, y, spawnFor, r, l);
	}

	// Internal tube/wall command, intended only to be used by the /load logic.  Again, no error checking occurs.
	// Format: /itw (Tube/Wall type) (X) (Y)
	else if (strcmp(word[0], "itw") == 0)
	{
		int type, x, y;
		char *c;
		type = strtol(word[1], &c, 0);
		x = strtol(word[2], &c, 0);
		y = strtol(word[3], &c, 0);
		TethysGame::CreateWallOrTube(x, y, -1, (map_id)type);
	}
	
}
