//
//  GMLAN.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//

#include "GMLAN.hpp"

#include <map>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <string>

#define PLAT_GEN_STAT	0x1F1

#define TRAN_ROT		  0x0C7
#define PLAT_CONF 		  0x4E9
#define TRANS_STAT_3 	   0x4C9
#define TRANS_STAT_2 	   0x1F5

#define ENGINE_GEN_STAT   0x1A1

#define ENGINE_GEN_STAT_1  0x0C9
#define ENGINE_GEN_STAT_4 0x4C1
#define ENGINE_GEN_STAT_2 0x3D1
#define ENGINE_GEN_STAT_3 0x3F9
 
#define ENGINE_GEN_STAT_5 0x4D1
#define FUEL_SYSTEM_2  0x1EF
#define VEHICLE_SPEED 0x3E9
 
 // derived from GMW8762
map<uint, string> knownPid = {
{ 0x0C1, "Driven Wheel Rotational Status"},
{ 0x0C5, "Non Driven Wheel Rotational Status"},
{ TRAN_ROT, "Transmission Output Rotational Status"},
{ ENGINE_GEN_STAT_1, "Engine General Status 1"},
{ 0x0F1, "Brake Apply Status"},
{ 0x0F9, "Transmission General Status 1"},
{ 0x150, "High Voltage Battery Information 1"},
{ 0x154, "High Voltage Battery Information 2"},
{ 0x158, "High Voltage Battery Information 3"},
{ 0x17D, "Antilock_Brake_and_TC_Status_HS"},
{ ENGINE_GEN_STAT, "Engine General Status"},
{ 0x1C3, "Engine Torque Status 2"},
{ 0x1C4, "Torque Request Status"},
{ 0x1C5, "Driver Intended Axle Torque Status"},
{ 0x1C7, "Chassis Engine Torque Request 1"},
{ 0x1C8, "Launch Control Request"},
{ 0x1CC, "Secondary Axle Status"},
{ 0x1CE, "Secondary Axle Control"},
{ 0x1CF, "Secondary Axle General Information"},
{ 0x1D0, "Front Axle Status"},
{ 0x1D1, "Rear Axle Status"},
{ 0x1E1, "Cruise Control Switch Status"},
{ 0x1E5, "Steering Wheel Angle"},
{ 0x1E9, "Chassis General Status 1"},
{ 0x1EA, "Alternative Fuel System Status"},
{ 0x1EB, "Fuel System Status"},
{ 0x1ED, "Fuel System Request"},
{ FUEL_SYSTEM_2, "Fuel System Request 2"},
{ PLAT_GEN_STAT, "Platform General Status"},
{ 0x1F3, "Platform Transmission Requests"},
{ TRANS_STAT_2, "Transmission General Status 2"},
{ 0x1F9, "PTO Command Data"},
{ 0x2B0, "Starter_Generator_Status_3"},
{ 0x2C3, "Engine Torque Status 3"},
{ 0x2CB, "Adaptive Cruise Axle Torque Request"},
{ 0x2F9, "Chassis General Status 2"},
{ 0x3C1, "Powertrain Immobilizer Data"},
{ 0x3C9, "Platform Immobilizer Data"},
{ ENGINE_GEN_STAT_2, "Engine General Status 2"},
{ 0x3E1, "Engine_BAS_Status_1"},
{ VEHICLE_SPEED, "Vehicle Speed and Distance"},
{ 0x3ED, "Vehicle_Limit_Speed_Control_Cmd"},
{ 0x3F1, "Platform Engine Control Request"},
{ 0x3F9, "Engine General Status 3"},
{ 0x3FB, "Engine Fuel Status"},
{ 0x451, "Gateway LS General Information"},
{ ENGINE_GEN_STAT_4, "Engine General Status 4"},
{ TRANS_STAT_3, "Transmission General Status 3"},
{ ENGINE_GEN_STAT_5, "Engine General Status 5"},
{ 0x4D9, "Fuel System General Status"},
{ 0x4E1, "Vehicle Identification Digits 10 thru 17"},
{ PLAT_CONF, "Platform Configuration Data"},
{ 0x4F1, "Powertrain Configuration Data"},
{ 0x4F3, "Powertrain Configuration Data 2"},
{ 0x772, "Diagnostic Trouble Code Information Extended"},
{ 0x77A, "Diagnostic Trouble Code Information Extended"},
{ 0x77F, "Diagnostic Trouble Code Information Extended"},
{ 0x3F3, "Power Pack General Status"},
{ 0x3F7, "Hybrid T emperature Status"},
{ 0x1D9, "Hybrid Balancing Request"},
{ 0x1DE, "Hybrid Battery General Status"}
};


string GMLAN::nameForFrame(can_frame_t frame){
	
	return "";
}

string GMLAN::descriptionForFrame(can_frame_t frame){
	string name = "";
	if(knownPid.count(frame.can_id)) {
		name = knownPid[frame.can_id];
	}

	return name;
}
