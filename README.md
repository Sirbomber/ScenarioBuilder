# ScenarioBuilder
 In-game scenario editor for Outpost 2, available as both a single player Colony Game and as a multiplayer mission.  The editor's primary purpose is to allow users to export the current unit setup on the map for use in a C++ or Python mission script, though the editor can also export a unit list that it can read in later, allowing players to create simple skirmishes without the need for any programming experience.
 
 The editor primarily operates through chat commands.  These are all of the commands the editor accepts, as well as (required) and [optional] parameters:
 
 Command list (all commands and arguments are case-sensitive):
	  /save												-- Generate code for the current setup on the local player's machine.  Does nothing for remote clients.
	  /load (PreviousOutputFileName.txt)				-- Loads a script file previously generated by the /save command.  Only the host may use this command.
														   This works by generating fake chat command packets from the host which contain spawning instructions.
														   Since the game can only process so many command packets per cycle, this may take awhile to fully load complex setups.
	  /cpp												-- Generates C++ code using TethysAPI for the current setup on the local player's machine.  Does nothing for remote clients.
	  /cpp_legacy                                       -- Generates C++ code using Outpost2DLL for the current setup on the local player's machine.  Does nothing for remote clients.
	  /python											-- Generate a Python script for the current setup on the local player's machine.  Does nothing for remote clients.
	  /dc (disType) (chance)							-- Sets the chance for the designated disaster (quake, storm, vortex, or meteor).  Disasters won't start spawning until /startLOS has been used.
	  /eden												-- Sets the source player to Eden
	  /plymouth											-- Sets the source player to Plymouth
	  /setplayer (0-6)									-- Changes the local player ID to the specified number.  Allows you to take control of other players.  Seems to work in multiplayer, but you can't take control of other humans.
	  /common (x) (y) [yield] [variant]					-- Spawn a common ore mine at the passed-in coordinates.  Yield and variant should both be between 0 (high/3bar) and 2 (low/1bar).
	  /rare (x) (y) [yield] [variant]					-- Spawn a rare ore mine at the passed-in coordinates.  Yield and variant should both be between 0 (high/3bar) and 2 (low/1bar).
	  /fumarole (x) (y)									-- Spawn a fumarole at the passed-in coordinates.
	  /magmavent (x) (y)								-- Spawn a magma vent at the passed-in coordinates.
	  /wreck (x) (y) [techID] [isDiscovered]            -- Spawns wreckage at the passed-in coordinates.  Defaults to "Tiger Speed Upgrade" and undiscovered.
	  /clear											-- Erases all units on the map.  Requires confirmation.  Attempting to delete mining beacons may crash OP2.
	  /changemap (mapname.map)							-- Loads the terrain of the specified map.  Cannot be used if any units currently exist on the map.  Only the host may use this command.
	  /spawn (Unit) (owner) (x) (y) [cargo] [amount]	-- Spawns the designated unit, optionally carrying the designated cargo, at the specified coordinates for the source player.
															You will not be allowed to give units "illegal" cargo (no ConVecs carrying Tigers, no Tigers armed with Command Centers, etc.).
															Amount is optional and only applies to Cargo Trucks.  Default is 1000 for resources, "Tiger Speed Upgrade" for wreckage, and EDWARD Satellite for starship components.
															Location is not checked before spawning, so you may get odd results if you do something the game doesn't expect!
															Valid options for Unit (and Cargo for ConVecs, as appropriate):
															 - CargoTruck
															 - ConVec
															 - Spider
															 - Scorpion
															 - Lynx
															 - Panther
															 - Tiger
															 - RoboSurveyor
															 - RoboMiner
															 - GeoCon
															 - Scout
															 - RoboDozer
															 - EvacuationTransport
															 - RepairVehicle
															 - Earthworker
															 - CommonOreMine
															 - RareOreMine
															 - GuardPost
															 - LightTower
															 - CommonStorage
															 - RareStorage
															 - Forum
															 - CommandCenter
															 - MHDGenerator
															 - Residence
															 - RobotCommand
															 - TradeCenter
															 - BasicLab
															 - MedicalCenter
															 - Nursery
															 - SolarPowerArray
															 - RecreationFacility
															 - University
															 - Agridome
															 - DIRT
															 - Garage
															 - MagmaWell
															 - MeteorDefense
															 - GeothermalPlant
															 - ArachnidFactory
															 - ConsumerFactory
															 - StructureFactory
															 - VehicleFactory
															 - StandardLab
															 - AdvancedLab
															 - Observatory
															 - ReinforcedResidence
															 - AdvancedResidence
															 - CommonOreSmelter
															 - Spaceport
															 - RareOreSmelter
															 - GORF
															 - Tokamak
														Valid options for weapons:
															 - AcidCloud
															 - EMP
															 - Laser
															 - Microwave
															 - RailGun
															 - RPG
															 - Starflare
															 - Supernova
															 - ESG
															 - Stickyfoam
															 - ThorsHammer
														Valid options for Cargo Truck cargo:
															 - Food
															 - CommonOre
															 - RareOre
															 - CommonMetal
															 - RareMetal
															 - CommonRubble
															 - RareRubble
															 - Spaceport (starship components and satellites - only use values 88 - 104, inclusive, when passing in an amount!)
															 - Garbage (wreckage - amount corresponds to tech ID - invalid tech IDs will crash!)
	/setres (resource type) (amount)					-- Gives the source player the specified quantity of the specified resources.
															Valid options for resource type:
															 - common
															 - rare
															 - food
															 - kids
															 - workers
															 - scientists
	/setTechLevel (tech level)							-- Gives all technologies whose tech level is less than the passed-in value.  Tech level 13 will give you all technologies.
														   Note that you can only give yourself techs this way, not take them away.
	/startLOS [keep resources]							-- Gives all players default starting resources and creates default Last One Standing victory/defeat conditions.  Only the host may use this command.
														   Passing in 1 for the keep resources parameter will override the default LOS resources setting.
														   Using this command disables all other commands aside from /save, /cpp, and /python.
														   Important note!  The standard LOS victory check is hardcoded to ignore AI players, so attempting a human vs CPU match will instantly end the mission after using this command.

AI Command List (work in progress):
		/ai mining (playerNo) (x1) (y1) (x2) (y2)		- Searches for existing common/rare ore mines and smelters within the specified area and sets up ore-hauling behavior.
														  Will fail if more than 1 mine of the same type exists within the specified area.
														  AI cannot currently build up its own mines and smelters; they must be pre-placed.
		/ai defenders (playerNo) (x1) (y1) (x2) (y2)	- Searches for combat units within the specified area and sets them up to automatically defend that area.
														  Due to internal limitations, only the first 32 units found will be added to the group.
		/ai reinforce (playerNo)						- Searches for Vehicle Factories owned by the specified player and sets them up to rebuild destroyed units from
														  any existing ore-hauling or defense unit groups.  This command should be called after all such groups
														  have already been defined.
