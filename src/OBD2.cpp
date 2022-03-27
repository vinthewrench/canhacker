//
//  OBD2.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/25/22.
//

#include "OBD2.hpp"
 
#define CAN_OBD_MASK 0x00000700U /* standard frame format (SFF) */

typedef struct {
	string_view  					title;
	string_view  				description;
	string_view  					units;
} valueSchema_t;


static map<uint16_t, valueSchema_t> _J2190schemaMap ={
	{0x1940, {"TRANS_TEMP",	"Transmission Temp",	"Unit.celsius"}},
	{0x132A, 	{"FUEL",	"FUEL?",	"string"}},
	{0x115C, 	{"OIL_PRESSURE",	"Oil Pressure?",	"Unit.kilopascal"}},
	{0x199A, 	{"GEAR",	"Current Gear",	"string"}},
 };


static map<uint8_t,  valueSchema_t> _schemaMap =
{
	{ 0x00,	{"PIDS_A",	"Supported PIDs [01-20]",	"bitarray"}},
	{ 0x01,	{"STATUS",	"Status since DTCs cleared",	"special"}},
	{ 0x02,	{"FREEZE_DTC",	"DTC that triggered the freeze frame",	"special"}},
	{ 0x03,	{"FUEL_STATUS",	"Fuel System Status",	"string"}},
	{ 0x04,	{"ENGINE_LOAD",	"Calculated Engine Load",	"Unit.percent"}},
	{ 0x05,	{"COOLANT_TEMP",	"Engine Coolant Temperature",	"Unit.celsius"}},
	{ 0x06,	{"SHORT_FUEL_TRIM_1",	"Short Term Fuel Trim - Bank 1",	"Unit.percent"}},
	{ 0x07,	{"LONG_FUEL_TRIM_1",	"Long Term Fuel Trim - Bank 1",	"Unit.percent"}},
	{ 0x08,	{"SHORT_FUEL_TRIM_2",	"Short Term Fuel Trim - Bank 2",	"Unit.percent"}},
	{ 0x09,	{"LONG_FUEL_TRIM_2",	"Long Term Fuel Trim - Bank 2",	"Unit.percent"}},
	{ 0x0A,	{"FUEL_PRESSURE",	"Fuel Pressure",	"Unit.kilopascal"}},
	{ 0x0B,	{"INTAKE_PRESSURE",	"Intake Manifold Pressure",	"Unit.kilopascal"}},
	{ 0x0C,	{"RPM",	"Engine RPM",	"Unit.rpm"}},
	{ 0x0D,	{"SPEED",	"Vehicle Speed",	"Unit.kph"}},
	{ 0x0E,	{"TIMING_ADVANCE",	"Timing Advance",	"Unit.degree"}},
	{ 0x0F,	{"INTAKE_TEMP",	"Intake Air Temp",	"Unit.celsius"}},
	{ 0x10,	{"MAF",	"Air Flow Rate (MAF)",	"Unit.grams_per_second"}},
	{ 0x11,	{"THROTTLE_POS",	"Throttle Position",	"Unit.percent"}},
	{ 0x12,	{"AIR_STATUS",	"Secondary Air Status",	"string"}},
	{ 0x13,	{"O2_SENSORS",	"O2 Sensors Present",	"special"}},
	{ 0x14,	{"O2_B1S1",	"O2: Bank 1 - Sensor 1 Voltage",	"Unit.volt"}},
	{ 0x15,	{"O2_B1S2",	"O2: Bank 1 - Sensor 2 Voltage",	"Unit.volt"}},
	{ 0x16,	{"O2_B1S3",	"O2: Bank 1 - Sensor 3 Voltage",	"Unit.volt"}},
	{ 0x17,	{"O2_B1S4",	"O2: Bank 1 - Sensor 4 Voltage",	"Unit.volt"}},
	{ 0x18,	{"O2_B2S1",	"O2: Bank 2 - Sensor 1 Voltage",	"Unit.volt"}},
	{ 0x19,	{"O2_B2S2",	"O2: Bank 2 - Sensor 2 Voltage",	"Unit.volt"}},
	{ 0x1A,	{"O2_B2S3",	"O2: Bank 2 - Sensor 3 Voltage",	"Unit.volt"}},
	{ 0x1B,	{"O2_B2S4",	"O2: Bank 2 - Sensor 4 Voltage",	"Unit.volt"}},
	{ 0x1C,	{"OBD_COMPLIANCE",	"OBD Standards Compliance",	"string"}},
	{ 0x1D,	{"O2_SENSORS_ALT",	"O2 Sensors Present (alternate)",	"special"}},
	{ 0x1E,	{"AUX_INPUT_STATUS",	"Auxiliary input status (power take off)",	"boolean"}},
	{ 0x1F,	{"RUN_TIME",	"Engine Run Time",	"Unit.second"}},
	{ 0x20,	{"PIDS_B",	"Supported PIDs [21-40]",	"bitarray"}},
	{ 0x21,	{"DISTANCE_W_MIL",	"Distance Traveled with MIL on",	"Unit.kilometer"}},
	{ 0x22,	{"FUEL_RAIL_PRESSURE_VAC",	"Fuel Rail Pressure (relative to vacuum)",	"Unit.kilopascal"}},
	{ 0x23,	{"FUEL_RAIL_PRESSURE_DIRECT",	"Fuel Rail Pressure (direct inject)",	"Unit.kilopascal"}},
	{ 0x24,	{"O2_S1_WR_VOLTAGE",	"02 Sensor 1 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x25,	{"O2_S2_WR_VOLTAGE",	"02 Sensor 2 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x26,	{"O2_S3_WR_VOLTAGE",	"02 Sensor 3 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x27,	{"O2_S4_WR_VOLTAGE",	"02 Sensor 4 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x28,	{"O2_S5_WR_VOLTAGE",	"02 Sensor 5 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x29,	{"O2_S6_WR_VOLTAGE",	"02 Sensor 6 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x2A,	{"O2_S7_WR_VOLTAGE",	"02 Sensor 7 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x2B,	{"O2_S8_WR_VOLTAGE",	"02 Sensor 8 WR Lambda Voltage",	"Unit.volt"}},
	{ 0x2C,	{"COMMANDED_EGR",	"Commanded EGR",	"Unit.percent"}},
	{ 0x2D,	{"EGR_ERROR",	"EGR Error",	"Unit.percent"}},
	{ 0x2E,	{"EVAPORATIVE_PURGE",	"Commanded Evaporative Purge",	"Unit.percent"}},
	{ 0x2F,	{"FUEL_LEVEL",	"Fuel Level Input",	"Unit.percent"}},
	{ 0x30,	{"WARMUPS_SINCE_DTC_CLEAR",	"Number of warm-ups since codes cleared",	"Unit.count"}},
	{ 0x31,	{"DISTANCE_SINCE_DTC_CLEAR",	"Distance traveled since codes cleared",	"Unit.kilometer"}},
	{ 0x32,	{"EVAP_VAPOR_PRESSURE",	"Evaporative system vapor pressure",	"Unit.pascal"}},
	{ 0x33,	{"BAROMETRIC_PRESSURE",	"Barometric Pressure",	"Unit.kilopascal"}},
	{ 0x34,	{"O2_S1_WR_CURRENT",	"02 Sensor 1 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x35,	{"O2_S2_WR_CURRENT",	"02 Sensor 2 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x36,	{"O2_S3_WR_CURRENT",	"02 Sensor 3 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x37,	{"O2_S4_WR_CURRENT",	"02 Sensor 4 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x38,	{"O2_S5_WR_CURRENT",	"02 Sensor 5 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x39,	{"O2_S6_WR_CURRENT",	"02 Sensor 6 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x3A,	{"O2_S7_WR_CURRENT",	"02 Sensor 7 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x3B,	{"O2_S8_WR_CURRENT",	"02 Sensor 8 WR Lambda Current",	"Unit.milliampere"}},
	{ 0x3C,	{"CATALYST_TEMP_B1S1",	"Catalyst Temperature: Bank 1 - Sensor 1",	"Unit.celsius"}},
	{ 0x3D,	{"CATALYST_TEMP_B2S1",	"Catalyst Temperature: Bank 2 - Sensor 1",	"Unit.celsius"}},
	{ 0x3E,	{"CATALYST_TEMP_B1S2",	"Catalyst Temperature: Bank 1 - Sensor 2",	"Unit.celsius"}},
	{ 0x3F,	{"CATALYST_TEMP_B2S2",	"Catalyst Temperature: Bank 2 - Sensor 2",	"Unit.celsius"}},
	{ 0x40,	{"PIDS_C",	"Supported PIDs [41-60]",	"bitarray"}},
	{ 0x41,	{"STATUS_DRIVE_CYCLE",	"Monitor status this drive cycle",	"special"}},
	{ 0x42,	{"CONTROL_MODULE_VOLTAGE",	"Control module voltage",	"Unit.volt"}},
	{ 0x43,	{"ABSOLUTE_LOAD",	"Absolute load value",	"Unit.percent"}},
	{ 0x44,	{"COMMANDED_EQUIV_RATIO",	"Commanded equivalence ratio",	"Unit.ratio"}},
	{ 0x45,	{"RELATIVE_THROTTLE_POS",	"Relative throttle position",	"Unit.percent"}},
	{ 0x46,	{"AMBIANT_AIR_TEMP",	"Ambient air temperature",	"Unit.celsius"}},
	{ 0x47,	{"THROTTLE_POS_B",	"Absolute throttle position B",	"Unit.percent"}},
	{ 0x48,	{"THROTTLE_POS_C",	"Absolute throttle position C",	"Unit.percent"}},
	{ 0x49,	{"ACCELERATOR_POS_D",	"Accelerator pedal position D",	"Unit.percent"}},
	{ 0x4A,	{"ACCELERATOR_POS_E",	"Accelerator pedal position E",	"Unit.percent"}},
	{ 0x4B,	{"ACCELERATOR_POS_F",	"Accelerator pedal position F",	"Unit.percent"}},
	{ 0x4C,	{"THROTTLE_ACTUATOR",	"Commanded throttle actuator",	"Unit.percent"}},
	{ 0x4D,	{"RUN_TIME_MIL",	"Time run with MIL on",	"Unit.minute"}},
	{ 0x4E,	{"TIME_SINCE_DTC_CLEARED",	"Time since trouble codes cleared",	"Unit.minute"}},
	{ 0x4F,	{"unsupported",	"unsupported"",	"}},
	{ 0x50,	{"MAX_MAF",	"Maximum value for mass air flow sensor",	"Unit.grams_per_second"}},
	{ 0x51,	{"FUEL_TYPE",	"Fuel Type",	"string"}},
	{ 0x52,	{"ETHANOL_PERCENT",	"Ethanol Fuel Percent",	"Unit.percent"}},
	{ 0x53,	{"EVAP_VAPOR_PRESSURE_ABS",	"Absolute Evap system Vapor Pressure",	"Unit.kilopascal"}},
	{ 0x54,	{"EVAP_VAPOR_PRESSURE_ALT",	"Evap system vapor pressure",	"Unit.pascal"}},
	{ 0x55,	{"SHORT_O2_TRIM_B1",	"Short term secondary O2 trim - Bank 1",	"Unit.percent"}},
	{ 0x56,	{"LONG_O2_TRIM_B1",	"Long term secondary O2 trim - Bank 1",	"Unit.percent"}},
	{ 0x57,	{"SHORT_O2_TRIM_B2",	"Short term secondary O2 trim - Bank 2",	"Unit.percent"}},
	{ 0x58,	{"LONG_O2_TRIM_B2",	"Long term secondary O2 trim - Bank 2",	"Unit.percent"}},
	{ 0x59,	{"FUEL_RAIL_PRESSURE_ABS",	"Fuel rail pressure (absolute)",	"Unit.kilopascal"}},
	{ 0x5A,	{"RELATIVE_ACCEL_POS",	"Relative accelerator pedal position",	"Unit.percent"}},
	{ 0x5B,	{"HYBRID_BATTERY_REMAINING",	"Hybrid battery pack remaining life",	"Unit.percent"}},
	{ 0x5C,	{"OIL_TEMP",	"Engine oil temperature",	"Unit.celsius"}},
	{ 0x5D,	{"FUEL_INJECT_TIMING",	"Fuel injection timing",	"Unit.degree"}},
	{ 0x5E,	{"FUEL_RATE",	"Engine fuel rate",	"Unit.liters_per_hour"}},
	{ 0x5F,	{"unsupported",	"unsupported",	""}}
};


 
inline  std::string hexDumpOBDData(canid_t can_id, uint8_t mode, uint8_t pid,
											  uint16_t len, uint8_t* data) {
	
	char      	 lineBuf[256] ;
	char       	 *p = lineBuf;
	bool showHex = true;
	bool showascii = false;
	uint16_t ext = 0;
	
	if(mode == 0x22){
		ext = (pid << 8) | data[0];
		data++;
		len--;
	}
	
	p += sprintf(p, "%03X (%02X %02X) [%d] ",  can_id, mode, pid, len);

	if(mode == 9 && pid == 2){
		showHex = false;
		showascii = true;
	}
	
	if(showHex){
		for (int i = 0; i < len; i++) p += sprintf(p,"%02X ",data[i]);
		for (int i = 7; i >  len ; i--) p += sprintf(p,"   ");
	}
	
	if(showascii){
		for (int i = 0; i < len; i++)  {
		uint8_t c = data[i] & 0xFF;
		if (c > ' ' && c < '~')
			*p++ = c ;
		else {
			*p++ = '.';
		}
	}
	}
	p += sprintf(p,"  ");
 
	if(mode == 1 || mode == 2){
		if(_schemaMap.count(pid)){
			auto entry = _schemaMap[pid];
			p += sprintf(p, "%s",  string(entry.title).c_str());
		}
	}
	else if(mode == 0x22){
		p += sprintf(p, "SAE J2190 Code 22 (%04X) ", ext);
		if(_J2190schemaMap.count(ext)){
			auto entry = _J2190schemaMap[ext];
			p += sprintf(p, "%s",  string(entry.title).c_str());
		}
	}
	else if(mode == 9 && pid == 2){
		p += sprintf(p,"  VIN number");
	}

	
	*p++ = 0;
	
	return string(lineBuf);
}

OBD2::OBD2(){
	_ecu_messages.clear();
}

void OBD2::registerSchema(FrameDB*){
	
}

void OBD2::reset() {
	_ecu_messages.clear();
}


void OBD2:: processFrame(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag){

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
			obd_state_t s;
	 
			s.rollingcnt = 1;
			s.total_len = ((frame.data[0] & 0x0f) | frame.data[1]) - 2;
			s.mode = frame.data[2] & 0x1F;
			s.pid = frame.data[3];
			memcpy(s.buffer, &frame.data[4], 4);
			s.current_len = 4;
			_ecu_messages[can_id] = s;
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

void OBD2::processOBDResponse(FrameDB* db,time_t when, eTag_t eTag,
										canid_t can_id,
									   uint8_t mode, uint8_t pid, uint16_t len, uint8_t* data){
	
	printf("%s\r\n",hexDumpOBDData(can_id, mode, pid,  len, data).c_str());

}



string OBD2::nameForFrame(can_frame_t frame){
	
	return "";
}

string OBD2::descriptionForFrame(can_frame_t frame){
	string name = "";
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
