//
//  Wranger2010.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//

#include "Wranger2010.hpp"
#include "FrameDB.hpp"

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



Wranger2010::Wranger2010(){
	
}

void Wranger2010::registerSchema(FrameDB* db){
	db->addSchema( _value_key_map[STEERING_ANGLE],
					  {"Steer", "Steering Angle",	FrameDB::DEGREES});

	db->addSchema( _value_key_map[MILEAGE],
					  {"Mileage", "Vehicle Mileage",	FrameDB::KM});

	db->addSchema( _value_key_map[KEY_POSITION],
					  {"Key", "Key Position",	FrameDB::STRING});

	db->addSchema( _value_key_map[FUEL_LEVEL],
					  {"Fuel", "Fuel Level",	FrameDB::PERCENT});

	db->addSchema( _value_key_map[DOORS],
					  {"Door", "Door State",	FrameDB::BINARY});

	db->addSchema( _value_key_map[CLOCK],
					  {"Clock", "Clock",	FrameDB::STRING});

}

void Wranger2010::processFrame(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag){
	switch(frame.can_id) {
			
		case 0x1E1:
		{
			uint16_t xx = (frame.data[2] <<8 | frame.data[3]);
			if (xx != 0xFFFF){
				float angle = xx - 4096. ;
				xx = angle * 0.4;
				db->updateValue(_value_key_map[STEERING_ANGLE], to_string(xx), when, eTag);
			};
			
 	 		}
			break;

		case 0x214:
		{
			int dist = 	(frame.data[0] << 12  | frame.data[1] <<8  | frame.data[2] );
			db->updateValue(_value_key_map[MILEAGE], to_string(dist), when, eTag);
		}
			break;

		case 0x21B:
		{
			float level = 	( frame.data[5] / 160.0 );
			db->updateValue(_value_key_map[FUEL_LEVEL], to_string(level), when, eTag);
		}
			break;

		case 0x244:
		{
			int doors = 	 frame.data[0] ;
			db->updateValue(_value_key_map[DOORS], to_string(doors), when, eTag);
		}
			break;

		case 0x3E6:
		{
			char str[10];
			sprintf (str, "%d:%02d:%02d", frame.data[0], frame.data[1],frame.data[2]);\
			db->updateValue(_value_key_map[CLOCK], string(str), when, eTag);
		}
			break;

			
		default:
		  break;
	};
	
}


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
 
