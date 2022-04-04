//
//  OBD2.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/25/22.
//

#include "OBD2.hpp"
#include "FrameDB.hpp"
#include "CanBusMgr.hpp"

#define CAN_OBD_MASK 0x00000700U /* standard frame format (SFF) */

typedef struct {
	string_view  						title;
	string_view  						description;
	FrameDB::valueSchemaUnits_t 	units;
} valueSchema_t;


static map<uint16_t, valueSchema_t> _J2190schemaMap ={
	{0x1940, 	{"OBD_TRANS_TEMP",	"Transmission Temperature",	FrameDB::DEGREES_C}},
	{0x132A, 	{"OBD_FUEL_??",	"FUEL?",					FrameDB::STRING}},
	{0x115C, 	{"OBD_OIL_PRESSURE",	"Oil Pressure?",	FrameDB::KPA}},
	{0x199A, 	{"OBD_TRANS_GEAR",	"Current Gear",			FrameDB::STRING}},
 };

static map<uint8_t, valueSchema_t> _service9schemaMap ={
	{ 0x02,	{"OBD_VIN",	"Vehicle Identification Number",	FrameDB::STRING}},
	{ 0x0A,  {"OBD_ECU_NAME", "ECU name",	FrameDB::STRING}},
	
	// for ECU name shift by and att CanID offset for ECU
	 //  (pid <<4 || (can_id & 0xf) )
	{ 0xA8,  {"OBD_ECU_0_NAME", "ECU 0 name",	FrameDB::STRING}},
	{ 0xA9,  {"OBD_ECU_1_NAME", "ECU 1 name",	FrameDB::STRING}},
	{ 0xAA,  {"OBD_ECU_2_NAME", "ECU 2 name",	FrameDB::STRING}},
	{ 0xAB,  {"OBD_ECU_3_NAME", "ECU 3 name",	FrameDB::STRING}},
	{ 0xAC,  {"OBD_ECU_4_NAME", "ECU 4 name",	FrameDB::STRING}},
	{ 0xAD,  {"OBD_ECU_5_NAME", "ECU 5 name",	FrameDB::STRING}},
	{ 0xAE,  {"OBD_ECU_6_NAME", "ECU 6 name",	FrameDB::STRING}},
	{ 0xAF,  {"OBD_ECU_7_NAME", "ECU 7 name",	FrameDB::STRING}},

};

static map<uint8_t,  valueSchema_t> _schemaMap =
{
	{ 0x00,	{"OBD_PIDS_A",	"Supported PIDs [01-20]",	FrameDB::BINARY}},
	{ 0x01,	{"OBD_STATUS",	"Status since DTCs cleared",	FrameDB::SPECIAL}},
	{ 0x02,	{"OBD_FREEZE_DTC",	"DTC that triggered the freeze frame",	FrameDB::SPECIAL}},
	{ 0x03,	{"OBD_FUEL_STATUS",	"Fuel System Status",	FrameDB::STRING}},
	{ 0x04,	{"OBD_ENGINE_LOAD",	"Calculated Engine Load",	FrameDB::PERCENT}},
	{ 0x05,	{"OBD_COOLANT_TEMP",	"Engine Coolant Temperature",	FrameDB::DEGREES_C}},
	{ 0x06,	{"OBD_SHORT_FUEL_TRIM_1",	"Short Term Fuel Trim - Bank 1",	FrameDB::FUEL_TRIM}},
	{ 0x07,	{"OBD_LONG_FUEL_TRIM_1",	"Long Term Fuel Trim - Bank 1",	FrameDB::FUEL_TRIM}},
	{ 0x08,	{"OBD_SHORT_FUEL_TRIM_2",	"Short Term Fuel Trim - Bank 2",	FrameDB::FUEL_TRIM}},
	{ 0x09,	{"OBD_LONG_FUEL_TRIM_2",	"Long Term Fuel Trim - Bank 2",	FrameDB::FUEL_TRIM}},
	{ 0x0A,	{"OBD_FUEL_PRESSURE",	"Fuel Pressure",	FrameDB::KPA}},
	{ 0x0B,	{"OBD_INTAKE_PRESSURE",	"Intake Manifold Pressure",	FrameDB::KPA}},
	{ 0x0C,	{"OBD_RPM",	"Engine RPM",	FrameDB::RPM}},
	{ 0x0D,	{"OBD_VEHICLE_SPEED",	"Vehicle Speed",	FrameDB::KPH}},
	{ 0x0E,	{"OBD_TIMING_ADVANCE",	"Timing Advance",	FrameDB::DEGREES}},
	{ 0x0F,	{"OBD_INTAKE_TEMP",	"Intake Air Temp",	FrameDB::DEGREES_C}},
	{ 0x10,	{"OBD_MAF",	"Air Flow Rate (MAF)",	FrameDB::GPS}},
	{ 0x11,	{"OBD_THROTTLE_POS",	"Throttle Position",	FrameDB::PERCENT}},
	{ 0x12,	{"OBD_AIR_STATUS",	"Secondary Air Status",	FrameDB::STRING}},
	{ 0x13,	{"OBD_O2_SENSORS",	"O2 Sensors Present",	FrameDB::SPECIAL}},
	{ 0x14,	{"OBD_O2_B1S1",	"O2: Bank 1 - Sensor 1 Voltage",	FrameDB::VOLTS}},
	{ 0x15,	{"OBD_O2_B1S2",	"O2: Bank 1 - Sensor 2 Voltage",	FrameDB::VOLTS}},
	{ 0x16,	{"OBD_O2_B1S3",	"O2: Bank 1 - Sensor 3 Voltage",	FrameDB::VOLTS}},
	{ 0x17,	{"OBD_O2_B1S4",	"O2: Bank 1 - Sensor 4 Voltage",	FrameDB::VOLTS}},
	{ 0x18,	{"OBD_O2_B2S1",	"O2: Bank 2 - Sensor 1 Voltage",	FrameDB::VOLTS}},
	{ 0x19,	{"OBD_O2_B2S2",	"O2: Bank 2 - Sensor 2 Voltage",	FrameDB::VOLTS}},
	{ 0x1A,	{"OBD_O2_B2S3",	"O2: Bank 2 - Sensor 3 Voltage",	FrameDB::VOLTS}},
	{ 0x1B,	{"OBD_O2_B2S4",	"O2: Bank 2 - Sensor 4 Voltage",	FrameDB::VOLTS}},
	{ 0x1C,	{"OBD_OBD_COMPLIANCE",	"OBD Standards Compliance",	FrameDB::STRING}},
	{ 0x1D,	{"OBD_O2_SENSORS_ALT",	"O2 Sensors Present (alternate)",FrameDB::SPECIAL}},
	{ 0x1E,	{"OBD_AUX_INPUT_STATUS",	"Auxiliary input status (power take off)",	FrameDB::BOOL}},
	{ 0x1F,	{"OBD_RUN_TIME",	"Engine Run Time",	FrameDB::SECONDS}},
	{ 0x20,	{"OBD_PIDS_B",	"Supported PIDs [21-40]",	FrameDB::BINARY}},
	{ 0x21,	{"OBD_DISTANCE_W_MIL",	"Distance Traveled with MIL on",	FrameDB::KM}},
	{ 0x22,	{"OBD_FUEL_RAIL_PRESSURE_VAC",	"Fuel Rail Pressure (relative to vacuum)",	FrameDB::KPA}},
	{ 0x23,	{"OBD_FUEL_RAIL_PRESSURE_DIRECT",	"Fuel Rail Pressure (direct inject)",	FrameDB::KPA}},
	{ 0x24,	{"OBD_O2_S1_WR_VOLTAGE",	"02 Sensor 1 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x25,	{"OBD_O2_S2_WR_VOLTAGE",	"02 Sensor 2 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x26,	{"OBD_O2_S3_WR_VOLTAGE",	"02 Sensor 3 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x27,	{"OBD_O2_S4_WR_VOLTAGE",	"02 Sensor 4 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x28,	{"OBD_O2_S5_WR_VOLTAGE",	"02 Sensor 5 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x29,	{"OBD_O2_S6_WR_VOLTAGE",	"02 Sensor 6 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x2A,	{"OBD_O2_S7_WR_VOLTAGE",	"02 Sensor 7 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x2B,	{"OBD_O2_S8_WR_VOLTAGE",	"02 Sensor 8 WR Lambda Voltage",	FrameDB::VOLTS}},
	{ 0x2C,	{"OBD_COMMANDED_EGR",	"Commanded EGR",	FrameDB::PERCENT}},
	{ 0x2D,	{"OBD_EGR_ERROR",	"EGR Error",	FrameDB::PERCENT}},
	{ 0x2E,	{"OBD_EVAPORATIVE_PURGE",	"Commanded Evaporative Purge",	FrameDB::PERCENT}},
	{ 0x2F,	{"OBD_FUEL_LEVEL",	"Fuel Level",	FrameDB::PERCENT}},
	{ 0x30,	{"OBD_WARMUPS_SINCE_DTC_CLEAR",	"Number of warm-ups since codes cleared",	FrameDB::INT}},
	{ 0x31,	{"OBD_DISTANCE_SINCE_DTC_CLEAR",	"Distance traveled since codes cleared",	FrameDB::KM}},
	{ 0x32,	{"OBD_EVAP_VAPOR_PRESSURE",	"Evaporative system vapor pressure",	FrameDB::PA}},
	{ 0x33,	{"OBD_BAROMETRIC_PRESSURE",	"Barometric Pressure",	FrameDB::KPA}},
	{ 0x34,	{"OBD_O2_S1_WR_CURRENT",	"02 Sensor 1 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x35,	{"OBD_O2_S2_WR_CURRENT",	"02 Sensor 2 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x36,	{"OBD_O2_S3_WR_CURRENT",	"02 Sensor 3 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x37,	{"OBD_O2_S4_WR_CURRENT",	"02 Sensor 4 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x38,	{"OBD_O2_S5_WR_CURRENT",	"02 Sensor 5 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x39,	{"OBD_O2_S6_WR_CURRENT",	"02 Sensor 6 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x3A,	{"OBD_O2_S7_WR_CURRENT",	"02 Sensor 7 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x3B,	{"OBD_O2_S8_WR_CURRENT",	"02 Sensor 8 WR Lambda Current",	FrameDB::MILLIAMPS}},
	{ 0x3C,	{"OBD_CATALYST_TEMP_B1S1",	"Catalyst Temperature: Bank 1 - Sensor 1",	FrameDB::DEGREES_C}},
	{ 0x3D,	{"OBD_CATALYST_TEMP_B2S1",	"Catalyst Temperature: Bank 2 - Sensor 1",	FrameDB::DEGREES_C}},
	{ 0x3E,	{"OBD_CATALYST_TEMP_B1S2",	"Catalyst Temperature: Bank 1 - Sensor 2",	FrameDB::DEGREES_C}},
	{ 0x3F,	{"OBD_CATALYST_TEMP_B2S2",	"Catalyst Temperature: Bank 2 - Sensor 2",	FrameDB::DEGREES_C}},
	{ 0x40,	{"OBD_PIDS_C",	"Supported PIDs [41-60]",	FrameDB::BINARY}},
	{ 0x41,	{"OBD_STATUS_DRIVE_CYCLE",	"Monitor status this drive cycle",	FrameDB::SPECIAL}},
	{ 0x42,	{"OBD_CONTROL_MODULE_VOLTAGE",	"Control module voltage",	FrameDB::VOLTS}},
	{ 0x43,	{"OBD_ABSOLUTE_LOAD",	"Absolute load value",	FrameDB::PERCENT}},
	{ 0x44,	{"OBD_COMMANDED_EQUIV_RATIO",	"Commanded equivalence ratio",	FrameDB::RATIO}},
	{ 0x45,	{"OBD_RELATIVE_THROTTLE_POS",	"Relative throttle position",	FrameDB::PERCENT}},
	{ 0x46,	{"OBD_AMBIANT_AIR_TEMP",	"Ambient air temperature",	FrameDB::DEGREES_C}},
	{ 0x47,	{"OBD_THROTTLE_POS_B",	"Absolute throttle position B",	FrameDB::PERCENT}},
	{ 0x48,	{"OBD_THROTTLE_POS_C",	"Absolute throttle position C",	FrameDB::PERCENT}},
	{ 0x49,	{"OBD_ACCELERATOR_POS_D",	"Accelerator pedal position D",	FrameDB::PERCENT}},
	{ 0x4A,	{"OBD_ACCELERATOR_POS_E",	"Accelerator pedal position E",	FrameDB::PERCENT}},
	{ 0x4B,	{"OBD_ACCELERATOR_POS_F",	"Accelerator pedal position F",	FrameDB::PERCENT}},
	{ 0x4C,	{"OBD_THROTTLE_ACTUATOR",	"Commanded throttle actuator",	FrameDB::PERCENT}},
	{ 0x4D,	{"OBD_RUN_TIME_MIL",	"Time run with MIL on",	FrameDB::MINUTES}},
	{ 0x4E,	{"OBD_TIME_SINCE_DTC_CLEARED",	"Time since trouble codes cleared",	FrameDB::MINUTES}},
	{ 0x4F,	{"OBD_unsupported",	"unsupported", FrameDB::UNKNOWN	}},
	{ 0x50,	{"OBD_MAX_MAF",	"Maximum value for mass air flow sensor",	FrameDB::GPS}},
	{ 0x51,	{"OBD_FUEL_TYPE",	"Fuel Type",	FrameDB::STRING}},
	{ 0x52,	{"OBD_ETHANOL_PERCENT",	"Ethanol Fuel Percent",	FrameDB::PERCENT}},
	{ 0x53,	{"OBD_EVAP_VAPOR_PRESSURE_ABS",	"Absolute Evap system Vapor Pressure",	FrameDB::KPA}},
	{ 0x54,	{"OBD_EVAP_VAPOR_PRESSURE_ALT",	"Evap system vapor pressure",	FrameDB::PA}},
	{ 0x55,	{"OBD_SHORT_O2_TRIM_B1",	"Short term secondary O2 trim - Bank 1",	FrameDB::PERCENT}},
	{ 0x56,	{"OBD_LONG_O2_TRIM_B1",	"Long term secondary O2 trim - Bank 1",	FrameDB::PERCENT}},
	{ 0x57,	{"OBD_SHORT_O2_TRIM_B2",	"Short term secondary O2 trim - Bank 2",	FrameDB::PERCENT}},
	{ 0x58,	{"OBD_LONG_O2_TRIM_B2",	"Long term secondary O2 trim - Bank 2",	FrameDB::PERCENT}},
	{ 0x59,	{"OBD_FUEL_RAIL_PRESSURE_ABS",	"Fuel rail pressure (absolute)",	FrameDB::KPA}},
	{ 0x5A,	{"OBD_RELATIVE_ACCEL_POS",	"Relative accelerator pedal position",	FrameDB::PERCENT}},
	{ 0x5B,	{"OBD_HYBRID_BATTERY_REMAINING",	"Hybrid battery pack remaining life",	FrameDB::PERCENT}},
	{ 0x5C,	{"OBD_OIL_TEMP",	"Engine oil temperature",	FrameDB::DEGREES_C}},
	{ 0x5D,	{"OBD_FUEL_INJECT_TIMING",	"Fuel injection timing",	FrameDB::DEGREES}},
	{ 0x5E,	{"OBD_FUEL_RATE",	"Engine fuel rate",	FrameDB::LPH}},
	{ 0x5F,	{"OBD_unsupported",	"unsupported",	FrameDB::UNKNOWN}}
};



 
//inline  std::string hexDumpOBDData(canid_t can_id, uint8_t mode, uint8_t pid, valueSchema_t* schema,
//											  uint16_t len, uint8_t* data) {
//
//	char      	 lineBuf[256] ;
//	char       	 *p = lineBuf;
//	bool showHex = true;
//	bool showascii = false;
//	uint16_t ext = 0;
//
//	if(mode == 0x22){
//		ext = (pid << 8) | data[0];
//		data++;
//		len--;
//	}
//
//	p += sprintf(p, "%03X %02X, %02X [%d] ",  can_id, mode, pid, len);
//
//	if(mode == 9 && pid == 2){
//		showHex = false;
//		showascii = true;
//	}
//
//	if(showHex){
//		for (int i = 0; i < len; i++) p += sprintf(p,"%02X ",data[i]);
//		for (int i = 7; i >  len ; i--) p += sprintf(p,"   ");
//	}
//
//	if(showascii){
//		for (int i = 0; i < len; i++)  {
//		uint8_t c = data[i] & 0xFF;
//		if (c > ' ' && c < '~')
//			*p++ = c ;
//		else {
//			*p++ = '.';
//		}
//	}
//	}
//	p += sprintf(p,"  ");
//
//	if(schema){
//		p += sprintf(p, "\t%s",  string(schema->description).c_str());
//	}
//
//	*p++ = 0;
//
//	return string(lineBuf);
//}

OBD2::OBD2(){
	_ecu_messages.clear();
}

void OBD2::registerSchema(FrameDB* db){
	
	for (auto it = _schemaMap.begin(); it != _schemaMap.end(); it++){
		valueSchema_t*  schema = &it->second;
		db->addSchema(schema->title,  {schema->title, schema->description, schema->units});
	}

	for (auto it = _service9schemaMap.begin(); it != _service9schemaMap.end(); it++){
		valueSchema_t*  schema = &it->second;
		db->addSchema(schema->title,  {schema->title, schema->description, schema->units});
	}

	for (auto it = _J2190schemaMap.begin(); it != _J2190schemaMap.end(); it++){
		valueSchema_t*  schema = &it->second;
		db->addSchema(schema->title,  {schema->title, schema->description, schema->units});
	}
}

void OBD2::reset() {
	_ecu_messages.clear();
}


void OBD2:: processFrame(FrameDB* db,string ifName, can_frame_t frame, time_t when, eTag_t eTag){

	canid_t can_id = frame.can_id & CAN_SFF_MASK;
	
	// is it an OBD2 request
	if((can_id & CAN_OBD_MASK) != 0x700) return;
 
	uint8_t frame_type = frame.data[0]>> 4;
	
	switch( frame_type){
		case 0: // single frame
		{
			// is it a OBD response? Only record responses
			if((frame.data[1] & 0x40) == 0x40){
				uint8_t len = frame.data[0] & 0x07;
				uint8_t mode = frame.data[1] & 0x3f;
				uint8_t pid = frame.data[2];
				
				processOBDResponse(db, when, eTag, can_id,
										 	mode, pid, len-2, &frame.data[3]);
			}
		}
			break;
			
		case 1: // first CAN message of a fragmented OBD-2 message
		{
			// if its one of ours we need to ask for more here..
			// send a flow control Continue To Send (CTS) frame

			
			CANBusMgr*	canBus = CANBusMgr::shared();
			if( canBus->sendFrame(ifName, can_id - 8 , {0x30}, NULL)){
				
				// only store te continue if we were successful.
				obd_state_t s;
		 
				s.rollingcnt = 1;
				s.total_len = ((frame.data[0] & 0x0f) | frame.data[1]) - 2;
				s.mode = frame.data[2] & 0x1F;
				s.pid = frame.data[3];
				memcpy(s.buffer, &frame.data[4], 4);
				s.current_len = 4;
				_ecu_messages[can_id] = s;
 			}
		}
			break;
		
		case 2: // consecutive message
		{
			auto it = _ecu_messages.find(can_id);
			if(it != _ecu_messages.end()){
				auto s = &it->second;
				uint8_t rollingcnt =  (frame.data[0] & 0x0f);
				if(s->rollingcnt != rollingcnt) break;
				s->rollingcnt++;
				
				uint16_t len = s->total_len - s->current_len;
				if(len > 7) len = 7;
				memcpy(s->buffer + s->current_len, &frame.data[1], len);
				s->current_len += len;
				
				if(s->current_len == s->total_len) {
					processOBDResponse(db, when, eTag,
											 can_id, s->mode, s->pid,
											  s->total_len, s->buffer);
					_ecu_messages.erase(it);
				}
			}
			break;
		}
			
		default: ;
			// not handled?
	
	}
 };

// value calculation and corrections
static string valueForData(canid_t can_id, uint8_t mode, uint8_t pid, uint16_t len, uint8_t* data){
	string value = string();

	if(mode == 0x22){	 // mode 22  J2190
		
		uint16_t ext = (pid << 8) | data[0];
		if(ext == 0x115C) {// oil pressure
			value = to_string(data[0] * 2 );
			}
	}
	else if(mode == 1 || mode == 2){
		switch(pid){
			case 0x42: //OBD_CONTROL_MODULE_VOLTAGE
				value = to_string( ((data[0] <<8 )| data[1]) / 1000.00) ;
				break;
			
			case 0x6:
			case 0x7:
			case 0x8:
			case 0x9:  //FUEL_TRIM
				value = to_string(  (data[0] * (100.0/128.0)) - 100. ) ;
				break;
		}
	}
	
	if(value.empty()){
		switch(len){
				case 1: value = to_string(data[0]); break;
				case 2: value = to_string((data[0] <<8 )| data[1]); break;
				case 3: value = to_string( (data[0] <<12)|(data[1] <<8) | data[2]); break;
				case 4: value = to_string( (data[0] <<16) || (data[1] <<12)|(data[2] <<8) | data[3]); break;
			default:  value =string( (char* )data, len); break;
			}
		}
	
	return value;;
}


void OBD2::processOBDResponse(FrameDB* db,time_t when, eTag_t eTag,
										canid_t can_id,
									   uint8_t mode, uint8_t pid, uint16_t len, uint8_t* data){
	
	valueSchema_t* schema = NULL;
 
	switch(mode){
		case 1:
		case 2:
			if(_schemaMap.count(pid)){
				schema = &_schemaMap[pid];
			}
			break;
			
		case 9:
			
			// ECU Names have an alternate Schema
			if(pid == 0x0A){
				pid =  (pid << 4 || (can_id & 0xf) );
			}
 
			if(_service9schemaMap.count(pid)){
				schema = &_service9schemaMap[pid];
			}
			break;
			
		case 0x22:
		{
			uint16_t ext = (pid << 8) | data[0];
			if(_J2190schemaMap.count(ext)){
				schema = &_J2190schemaMap[ext];
			}
			len--;
			data++;
		}
	}
	
 
	string value = valueForData(can_id, mode,pid,len, data);
	db->updateValue(schema->title  ,value,when, eTag);
}

 
string OBD2::descriptionForFrame(can_frame_t frame){
	string name = "";
	
	canid_t can_id = frame.can_id & CAN_SFF_MASK;
	
	// is it an OBD2 request
	if((can_id & CAN_OBD_MASK) == 0x700) {
		
		if(can_id == 0x7DF)
			name = string("OBD ALL");
		else if(can_id >= 0x7e0 && can_id <= 0x7E7)
			name = string("OBD REQ: ") + to_string(can_id - 0x7e0) ;
		else 	if(can_id >= 0x7e8 && can_id <= 0x7EF){
			name = string("OBD RES: ") + to_string(can_id - 0x7e8);
		}
	}

	
 	return name;
}

 


/*
 7E2 (01 0F) [1] 99                     INTAKE_TEMP
 7E8 (01 46) [1] 32                     AMBIANT_AIR_TEMP
 7E8 (01 05) [1] 73                     COOLANT_TEMP
 7E8 (01 0C) [2] 09 56                  RPM
 7E8 (01 0D) [1] 22                     SPEED
 7E8 (01 07) [1] 6B                     LONG_FUEL_TRIM_1
 7E8 (01 09) [1] 6F                     LONG_FUEL_TRIM_2
 7E8 (01 04) [1] 44                     ENGINE_LOAD
 7E8 (01 10) [2] 02 7C                  MAF
 7E8 (01 42) [2] 30 A3                  CONTROL_MODULE_VOLTAGE
 7E8 (01 1F) [2] 00 47                  RUN_TIME
 7EA (22 19) [1] 3D                     SAE J2190 Code 22 (1940) TRANS_TEMP
 7EA (22 13) [2] 06 61                  SAE J2190 Code 22 (132A) FUEL
 7E8 (22 11) [1] 5A                     SAE J2190 Code 22 (115C) OIL_PRESSURE
 7EA (22 19) [1] 01                     SAE J2190 Code 22 (199A) GEAR
 7E8 (09 02) [18] .1J4BA6H19AL220060    VIN number
 7E8 (09 02) [18] .3FADP4FJ2BM113913    VIN number
 OK>


 */
