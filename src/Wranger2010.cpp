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
#include "Utils.hpp"

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
	{ 0x514,	""},
	{ 0x516,	"Radio ODB RESP"},
	{ 0x6B0,	"Radio ODB REQ"},
	
	
};



typedef  enum  {
	STEERING_ANGLE,
	VEHICLE_DISTANCE,
	KEY_POSITION,
	FUEL_LEVEL,
	DOORS,
	DOORS_LOCK,
	CLOCK,
	RPM,
	VIN,
} value_keys_t;

typedef FrameDB::valueSchema_t valueSchema_t;

static map<value_keys_t,  valueSchema_t> _schemaMap = {
	{STEERING_ANGLE,		{"JK_STEERING_ANGLE",			"Steering Angle",							FrameDB::DEGREES}},
	{VEHICLE_DISTANCE,	{"JK_VEHICLE_DISTANCE",			"Vehicle Distance Driven",				FrameDB::KM}},
	{KEY_POSITION,			{"JK_KEY_POSITION",				"Ignition Key Position",				FrameDB::STRING}},
	{FUEL_LEVEL,			{"JK_FUEL_LEVEL",					"Fuel Level",								FrameDB::PERCENT}},
	{DOORS,					{"JK_DOORS",						"Door State",								FrameDB::BINARY}},
	{DOORS_LOCK,			{"JK_DOORS_LOCKED",				"Door Lock",								FrameDB::BOOL}},
	{CLOCK,					{"JK_CLOCK",						"Clock Time",								FrameDB::STRING}},
	{RPM,						{"JK_ENGINE_RPM",					"Engine RPM",								FrameDB::RPM}},
	{VIN,						{"JK_VIN",							"Vehicle Identification Number",		FrameDB::STRING}},
};



  static map<uint8_t,  valueSchema_t> _radio_schemaMap = {
	  {0x09,		{"JKR_09",			"Radio PID 09",	 FrameDB::DATA}},
	  {0x0E,		{"JKR_0E",			"Radio PID 0E",	 FrameDB::DATA}},
	  {0x10,		{"JKR_10",			"Radio PID 10",	 FrameDB::DATA}},
	  {0x11,		{"JKR_11",			"Radio PID 11",	 FrameDB::DATA}},
	  {0x12,		{"JKR_12",			"Radio PID 12",	 FrameDB::DATA}},
	  {0x18,		{"JKR_18",			"Radio PID 18",	 FrameDB::DATA}},
	  {0x30,		{"JKR_30",			"Radio PID 30",	 FrameDB::DATA}},
	  {0x34,		{"JKR_34",			"Radio PID 34",	 FrameDB::DATA}},
	  {0x35,		{"JKR_35",			"Radio PID 35",	 FrameDB::DATA}},
	  {0x36,		{"JKR_36",			"Radio PID 36",	 FrameDB::DATA}},
	  {0x44,		{"JKR_VIN",			"Radio PID VIN",	 FrameDB::STRING}},

	  {0x49,		{"JKR_49",			"Radio PID 49",	 FrameDB::DATA}},
	  {0x50,		{"JKR_50",			"Radio PID 50",	 FrameDB::DATA}},
	  {0x52,		{"JKR_52",			"Radio PID 52",	 FrameDB::DATA}},
	  {0xEA,		{"JKR_EA",			"Radio PID EA",	 FrameDB::DATA}},
 };



Wranger2010::Wranger2010(){
	reset();
}

void Wranger2010::reset(){
	_VIN.clear();
	_multi_frame.clear();
}

void Wranger2010::registerSchema(FrameDB* db){
	
	_db = db;
	for (auto it = _schemaMap.begin(); it != _schemaMap.end(); it++){
		valueSchema_t*  schema = &it->second;
		db->addSchema(schema->title,  {schema->title, schema->description, schema->units});
	}
 
	for (auto it = _radio_schemaMap.begin(); it != _radio_schemaMap.end(); it++){
		valueSchema_t*  schema = &it->second;
		db->addSchema(schema->title,  {schema->title, schema->description, schema->units});
	}

	db->registerFrameDecoder("can0", 0x6B0, processRadioFrameWrapper, this);
	db->registerFrameDecoder("can0", 0x516, processRadioFrameWrapper, this);
	//	db->registerFrameDecoder("can0", 0x53E, processRadioFrameWrapper, this);
	//	db->registerFrameDecoder("can0", 0x7F0, processRadioFrameWrapper, this);
}

string_view Wranger2010::schemaKeyForValueKey(int valueKey) {
	
	auto key = static_cast<value_keys_t>( valueKey);
	valueSchema_t*  schema = &_schemaMap[key];
	return schema->title;
}

void Wranger2010::processFrame(FrameDB* db,string ifName, can_frame_t frame, time_t when){
	switch(frame.can_id) {
			
		case 0x1E1:
		{
			uint16_t xx = (frame.data[2] <<8 | frame.data[3]);
			if (xx != 0xFFFF){
				float angle = xx - 4096. ;
				angle = angle * 0.4;
				db->updateValue(schemaKeyForValueKey(STEERING_ANGLE), to_string((int)angle), when);
			};
			
		}
			break;
			
		case 0x20B:		//"Key Position"
		{
			string value;
			uint8_t pos =  frame.data[0];
			switch (pos) {
				case 0x00:
					value = "No Key";
					break;
					
				case 0x01:
					value = "OFF";
					break;
					
				case 0x61:
					value = "ACC";
					break;
					
				case 0x81:
					value = "RUN";
					break;
					
				case 0xA1:
					value = "START";
					break;
					
				default:
					break;
			}
			
			if(!value.empty()){
				db->updateValue(schemaKeyForValueKey(KEY_POSITION), value, when);
			}
		}
			break;
			
		case 0x214:	//Distance
		{
			uint32_t dist = 	(frame.data[0] << 16  | frame.data[1] <<8  | frame.data[2] );
			if(dist != 0xffffff)
				db->updateValue(schemaKeyForValueKey(VEHICLE_DISTANCE), to_string(dist), when);
		}
			break;
			
		case 0x21B:	//Fuel level
		{
			float level = 	( (frame.data[5]  * 100.) / 160.0 );
			db->updateValue(schemaKeyForValueKey(FUEL_LEVEL), to_string(level), when);
		}
			break;
			
		case 0x244: //Door Status
		{
			int doors = 	 frame.data[0] ;
			db->updateValue(schemaKeyForValueKey(DOORS), to_string(doors), when);
			
			int locks = 	 frame.data[4] ;
			if(locks & 0x80)
				db->updateValue(schemaKeyForValueKey(DOORS_LOCK), to_string(false), when);
			else if(locks & 0x08)
				db->updateValue(schemaKeyForValueKey(DOORS_LOCK),  to_string(true), when);
			
		}
			break;
			
		case 0x2CE: // RPM
		{
			uint16_t xx = (frame.data[0] <<8 | frame.data[1]);
			if (xx != 0xFFFF){
				xx *= 4;
				db->updateValue(schemaKeyForValueKey(RPM), to_string(xx), when);
			};
		}
			break;
			
		case 0x3E6: //Clock Time Display
		{
			char str[10];
			sprintf (str, "%d:%02d:%02d", frame.data[0], frame.data[1],frame.data[2]);
			db->updateValue(schemaKeyForValueKey(CLOCK), string(str), when);
		}
			break;
			
		case 0x219: // JK VIN number
		{
			// this repeats with first byte as sequence number
			// --  once we get it stop updating
			
			static int stage = 0;
			
			if(_VIN.empty()) stage = 0;
			
			if (stage == 3) break;
			uint8_t b0 = frame.data[0];
			
			switch (stage) {
				case 0:
					if(b0 == 0){
						_VIN.append((char *)&frame.data[1], 7);
						stage++;
					}
					break;
					
				case 1:
					if(b0 == 1){
						_VIN.append((char *)&frame.data[1], 7);
						stage++;
					}
					break;
					
				case 2:
					if(b0 == 2){
						_VIN.append((char *)&frame.data[1], 7);
						db->updateValue(schemaKeyForValueKey(VIN), _VIN, when);
						stage++;
					}
					break;
					
			}
		}
			break;
			
			//		case 0x6B0:
			//		case 0x516:
			//		case 0x53E:
			//			// internal radio codes.
			//			processPrivateFrame(db,ifName,frame,when);
			//			break;
			
			
		default:
			break;
	};
	
}





string Wranger2010::descriptionForFrame(can_frame_t frame){
	string name = "";
	if(knownPid.count(frame.can_id)) {
		name = knownPid[frame.can_id];
	}
	
	return name;
}

void Wranger2010::processRadioFrameWrapper(void* context,
														 string ifName, canid_t can_id,
														 can_frame_t frame, unsigned long timeStamp){
	Wranger2010* d = (Wranger2010*)context;
	
	d->processRadioFrame(ifName, can_id, frame, timeStamp);
}


void Wranger2010::processRadioFrame(string ifName, canid_t can_id, can_frame_t frame, unsigned long timeStamp){
	
	uint8_t frame_type = frame.data[0]>> 4;
	
	//ISO 15765-2
	
	switch( frame_type){
		case 0: // single frame
		{
			// is it a OBD response? Only record responses
			//			if((frame.data[1] & 0x40) == 0x40){
			uint8_t len = frame.data[0] & 0x07;
			bool REQ = (frame.data[1] & 0x40)  == 0 ;
			
			uint8_t service_id = REQ?frame.data[1]: frame.data[1] & 0x3f;
			
			if(REQ){
				processPrivateODB(_db, timeStamp, can_id,
										REQ,  service_id,  len -1 , &frame.data[2]);
				
			}
			else {
				processPrivateODB(_db, timeStamp, can_id,
										REQ,  service_id,  len , &frame.data[2]);
				
			}
			
			//					}
		}
			break;
			
		case 1: // first CAN message of a fragmented OBD-2 message
		{
			// if its one of ours we need to ask for more here..
			// send a flow control Continue To Send (CTS) frame
			
			//				if(canBus->isReadingFile()
			//					||  canBus->sendFrame(ifName, can_id - 8 , {0x30, 0x00, 0x0A}, NULL))
			{
				
				// only store te continue if we were successful.
				obd_state_t s;
				
				s.rollingcnt = 1;
				s.total_len = (((frame.data[0] & 0x0f) <<4 )| frame.data[1]) - 1;
				s.service_id = frame.data[2] & 0x3F;
				s.pid = frame.data[3];
				memcpy(s.buffer, &frame.data[3], 5);
				s.current_len = 5;
				_multi_frame[can_id] = s;
			}
		}
			break;
			
		case 2: // consecutive message
		{
			auto it = _multi_frame.find(can_id);
			if(it != _multi_frame.end()){
				auto s = &it->second;
				uint8_t rollingcnt =  (frame.data[0] & 0x0f);
				if(s->rollingcnt != rollingcnt) break;
				s->rollingcnt = (s->rollingcnt+1) & 0x0f;  // MODULO 16
				
				uint16_t len = s->total_len - s->current_len;
				if(len > 7) len = 7;
				memcpy(s->buffer + s->current_len, &frame.data[1], len);
				s->current_len += len;
				
				if(s->current_len == s->total_len) {
					processPrivateODB(_db, timeStamp,
											can_id, false,  s->service_id,
											s->total_len, s->buffer);
					_multi_frame.erase(it);
				}
			}
			break;
		}
		case 3:  // ignore flow control Continue To Send (CTS) frame
			break;
		default: ;
			// not handled?
			
	}
}

void   Wranger2010::processPrivateODB(FrameDB* db,time_t when,
												  canid_t can_id,
												  bool isRequest,  uint8_t service_id,
												  uint16_t len, uint8_t* data){

 	//	if(!isRequest){
	//
	//	}
	
	//	db->addSchema(schema->title,  {schema->title, schema->description, schema->units});
	
	
	if(service_id == 0x3e){
		
		/*
		 weird request --
		 
		 REQ:     0x3e, 0x01
		 RSP: 		0x3E, 0x43
		 
		 */
		return;
	}
	
	if(! (service_id == 0x21 || service_id == 0x1A)){
		printf("\nStrange ID %3x: %s (%02X) \n",
				 can_id, isRequest?"REQ":"RSP",  service_id);
		return;
	}
	
	// the rest look like ODB2
	
	
	if(len > 0){
		uint8_t pid = data[0];
		len--;
		data++;
		
		if(isRequest) return;
	 
		// check for radio VIN
		if(pid == 0x44){
			static int stage = 0;
			
			static char VIN[22] = {0};
		 
			if(stage == 0)
				memset(VIN, 0, sizeof(VIN));
			
			// bad value
			uint8_t b0 = data[7];
		
			switch (b0) {
				case 0:
					memcpy( &VIN[0], &data[0], 7);
					stage++;
					break;
					
				case 1:
					memcpy( &VIN[7], &data[0], 7);
					stage++;
					break;
					
				case 2:
					memcpy( &VIN[14], &data[0], 7);
					stage++;
					break;
			}
			
			if(stage == 3){
				valueSchema_t*  schema = &_radio_schemaMap[pid];
				db->updateValue(schema->title, 	string(VIN), when);
				stage = 0;
 			}

 		}
 		else if(_radio_schemaMap.count(pid)){
			valueSchema_t*  schema = &_radio_schemaMap[pid];
			
			string value = hexStr(data,len);
			db->updateValue(schema->title, value, when);
			
		}
	 
//
//		printf("%02X  ",  pid);
//
//		if(len > 0){
//			printf("%2d: ", len);
//			for(int i = 0; i < len; i++)
//				printf("%02x ", data[i]);
//
//			if(len > 8){
//				printf("\r\n\t\t|");
//				for(int i = 0; i < len; i++){
//					uint8_t c =  data[i];
//					if (c > ' ' && c < '~')
//						printf("%c", data[i]);
//					else {
//						printf(".");
//					}
//				}
//				printf("|");
//
//			}
//
//		}
//		printf("\r\n");
//
	}
	
	
}
