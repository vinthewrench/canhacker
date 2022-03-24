//
//  Wranger2010.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//

#include "Wranger2010.hpp"

#include <map>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <string>

static map<uint, string> knownPid = {

	{ 0x142,	""},
	{ 0x1A5,	""},
	{ 0x1E1,	"Steering Angle "},
	{ 0x1E7,	""},
	{ 0x208,	"Lights control"},
	{ 0x20B,	"Key Position"},
	{ 0x20E,	"Transmission/ brake"},
	{ 0x211,	"Wheel Speed and Distance"},
	{ 0x214,	"Distance"},
	{ 0x217,	"Fuel Related"},
	{ 0x219,	"VIN Number"},
	{ 0x21B,	"Fuel level"},
	{ 0x21D,	""},
	{ 0x21E,	""},
	{ 0x21F,	""},
	{ 0x221,	""},
	{ 0x244,	"Door Status"},
	{ 0x249,	"SKREEM related "},
	{ 0x25F,	""},
	{ 0x270,	"Climate Control switch"},
	{ 0x283,	""},
	{ 0x286,	"Parking brake?"},
	{ 0x290,	""},
	{ 0x291,	"Radio Mode"},
	{ 0x292,	"Temperature + Throttle?? / voltage"},
	{ 0x293,	"Radio Station"},
	{ 0x295,	"Radio Show Title"},
	{ 0x2A8,	"Wiper switch"},
	{ 0x2B0,	"Switch Panel"},
	{ 0x2CA,	""},
	{ 0x2CE,	"RPM"},
	{ 0x2D2,	"Transfer Case / Seat Belts?"},
	{ 0x2D3,	""},
	{ 0x2D6,	""},
	{ 0x2D9,	"power on check?"},
	{ 0x2DA,	""},
	{ 0x2DB,	""},
	{ 0x2DD,	""},
	{ 0x2DE,	""},
	{ 0x2DF,	""},
	{ 0x2E1,	"Headlight Switch"},
	{ 0x2E3,	""},
	{ 0x2E5,	"Rear Wiper"},
	{ 0x2E7,	"Parking Brake"},
	{ 0x2E9,	"Set time on SKREEM"},
	{ 0x2EB,	"Temperature Related"},
	{ 0x308,	"Dimmer Switch"},
	{ 0x348,	""},
	{ 0x392,	""},
	{ 0x3A3,	"Steering wheel radio control"},
	{ 0x3B0,	""},
	{ 0x3B3,	""},
	{ 0x3D9,	"Radio Settings broadcast"},
	{ 0x3E6,	"Clock Time Display"},
	{ 0x3E9,	""},
	{ 0x402,	"heartbeat 1"},
	{ 0x414,	"heartbeat 2"},
	{ 0x416,	"heartbeat 3"},
	{ 0x43E,	"heartbeat 4"},
	{ 0x514,	""} };

string Wranger2010::nameForFrame(can_frame_t frame){
	
	return "";
}

string Wranger2010::descriptionForFrame(can_frame_t frame){
	string name = "";
	if(knownPid.count(frame.can_id)) {
		name = knownPid[frame.can_id];
	}

	return name;
}
