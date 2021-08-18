#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Outpost2DLL.h>
#include <OP2Helper.h>
#include <HFL.h>
#include <Patches.h>
#include <DisasterCreator\DisasterCreator.h>

#include <OP2Types.h>
#include <Outpost2App.h>

#include "ChatCommands.h"

HINSTANCE hInstDLL = NULL;

#ifdef _SOLO
Export char MapName[] = "tutorial.map";
Export char LevelDesc[] = "Editor";
Export char TechtreeName[] = "multitek.txt";
Export AIModDesc DescBlock = { Colony, 6, 12, 0 };
#else
Export char MapName[] = "tutorial.map";
Export char LevelDesc[] = "6P, LoS, 'Editor (Multiplayer)'";
Export char TechtreeName[] = "multitek.txt";
Export AIModDesc DescBlock = { MultiLastOneStanding, 6, 12, 0 };
Export int DescBlockEx[] = { 5, 0, 0, 0, 0, 0, 0, 0 };
#endif

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) 
	{
		DisableThreadLibraryCalls(hinstDLL);
		hInstDLL = hinstDLL;

		if (HFLInit() == HFLCANTINIT
			|| !SetMissionEndHook(true, NULL)
			|| !SetMessageHook(true, ProcessChatCommands))
		{
			return false;
		}
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		SetMessageHook(false, NULL);
		SetNukePatch(false);
		SetMissionEndHook(false, NULL);

		HFLCleanup();
	}

	return TRUE;
}

DisasterCreator DC;

Export int InitProc()
{
#ifndef _SOLO
	TethysGame::ForceMoraleGood(-1);
	
	TethysGame::SetDaylightEverywhere(1);
	if (TethysGame::UsesDayNight())
	{
		TethysGame::SetDaylightEverywhere(0);
		GameMap::SetInitialLightLevel(64);
		TethysGame::SetDaylightMoves(1);
	}
#else
	TethysGame::ForceMoraleGreat(-1);
#endif

	for (int i = 0; i < 6; i++)
	{
		Player[i].CenterViewOn(2 + 31, 2 - 1);
		ExtPlayer[i].SetSatelliteCount(mapEDWARDSatellite, 1);
		Player[i].SetFoodStored(666666);
		Player[i].SetScientists(200);
#ifdef _SOLO
		Player[i].GoEden();
		//Player[i].SetTechLevel(13);
		
#endif
	}

	// Set rare ore mine's tech requirement to Command Center tech (always available) so players can see rare ore mines even without Rare Ore Processing
	for (int i = 0; i < ((OP2Research*)researchObj)->numTechs; ++i)
	{
		if (((OP2Research*)researchObj)->techInfos[i]->techID == 2103)
		{
			unitInfoArray[mapRareOreMine]->techId = i;
			break;
		}
	}

	// Set default values for Disaster Creator
	DC.SetMinimumWait(3700);
	DC.SetIgnoreChance(7);
	DC.SetDisasterTypeWeight(disNone, 100);
	DC.SetDisasterPowerWeight(pwrLow, 40);
	DC.SetDisasterPowerWeight(pwrMedium, 40);
	DC.SetDisasterPowerWeight(pwrHigh, 16);
	DC.SetDisasterPowerWeight(pwrApocalyptic, 4);
	DC.SetDisasterTargetWeight(trgPlayer, 100);

	DC.SetCallbackTrigger("DisasterCreator_Callback", 200);

	return 1; // return 1 if OK; 0 on failure
}

Export void DisasterCreator_Callback()
{
	DC.RollRandom();
	DC.CheckVolcanoes();
}

// ------------------------------------------------------------------
// The usual OP2 stuff
Export void AIProc()
{
	//
}

Export void __cdecl GetSaveRegions(struct BufferDesc &bufDesc)
{
	//
}

// I use "None" instead of "NoResponseToTrigger" because it's shorter
Export void None()
{
	//
}

// But I need this because OP2Helper references this for the default LOS setup.
Export void NoResponseToTrigger()
{
}