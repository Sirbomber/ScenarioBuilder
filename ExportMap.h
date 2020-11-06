#pragma once

enum outputType
{
	cpp = 0,
	python = 1,
	script = 2,

	unknown = -1
};

// Export data
void Save();											// Exports chat commands to recreate the current setup.  Commands can be replayed by the /load command.
void GenerateCpp();										// Generates C++ code to recreate the current setup.
void GeneratePython();									// Generates a Python script to recreate the current setup.  Not implemented.

// C++ code generation
void GenerateUnitLists(outputType saveAs=cpp);			// Generate code to reproduce the current unit setup
void GenerateTubesAndWallsLists(outputType saveAs=cpp);	// Generate code to reproduce the walls and tubes currently built around the map

// Helper functions
std::string MapIdToString(map_id type);					// Convert internal map id values to external strings, to make generated code easier to read (ex 2 becomes "mapConVec")
std::string TruckCargoToString(Truck_Cargo cargo);		// As above, but for truck cargo types
int NormalizeRotation(char r);							// Convert internal rotation values to the 0-7 value TethysGame::CreateUnit expects
std::string MapIdToChatId(map_id type);					// Convert internal map id values to chat command string
std::string TruckCargoToChatId(Truck_Cargo cargo);		// As above, but for truck cargo types
map_id ChatIdToMapId(const char* type);					// Convert the string used to reference units in chat commands to an internal map id.
Truck_Cargo ChatIdToTruckCargo(const char* type);		// As above, but for truck cargo types

void LoadScript(const char* filename);					// Load saved script and replay chat commands.