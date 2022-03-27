//
//  GMLAN.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//

#pragma once

#include "CanProtocol.hpp"
#include <map>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <string>

class GMLAN: public CanProtocol {
	
public:
   
	typedef  enum  {
		ENGINE_RPM,
		ENGINE_RUNNING,
		FUEL_CONSUPTION,
		THROTTLE_POS,
		FAN_SPEED,
		TEMP_COOLANT,
		TEMP_TRANSMISSION,
		PRESSURE_OIL,
		VEHICLE_SPEED,
		
		MASS_AIR_FLOW,
		BAROMETRIC_PRESSURE,
		TEMP_AIR_INTAKE,
		TEMP_AIR_AMBIENT,
		TRANS_GEAR,
	 
		/// Not a valid value; this is the number of members in this enum
		count,
		// helpers for iterating over the enum
		begin = 0,
		end = count,
	} value_keys_t;
	
	
	GMLAN();
	
	virtual void registerSchema(FrameDB*);

	virtual void processFrame(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);
	virtual string nameForFrame(can_frame_t frame);
	virtual string descriptionForFrame(can_frame_t frame);
	

	map<value_keys_t,  string_view> _value_key_map = {
		{ENGINE_RPM , 			"GM_ENGINE_RPM"},
		{ENGINE_RUNNING , 	"GM_ENGINE_RUNNING"},
		{FUEL_CONSUPTION, 	"GM_FUEL_CONSUPTION"},
		{THROTTLE_POS, 		"GM_THROTTLE_POS"},
		{FAN_SPEED, 			"GM_FAN_SPEED"},
		{TEMP_COOLANT, 		"GM_TEMP_COOLANT"},
		{TEMP_TRANSMISSION, 	"GM_TEMP_TRANSMISSION"},
		{PRESSURE_OIL,			"GM_PRESSURE_OIL"},
		{VEHICLE_SPEED,		"GM_VEHICLE_SPEED"},
		{MASS_AIR_FLOW, 		"GM_MASS_AIR_FLOW"},
		{BAROMETRIC_PRESSURE, 	"BAROMETRIC_PRESSURE"},
		{TEMP_AIR_INTAKE, 	"TEMP_AIR_INTAKE"},
		{TEMP_AIR_AMBIENT, 	"TEMP_AIR_AMBIENT"},
		{TRANS_GEAR, 			"TRANS_GEAR"},
		
	};
	
		
private:
	string_view keyForValueKey(int valueKey);

	// specific updates
	void processPlatGenStatus(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processEngineGenStatus(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processEngineGenStatus1(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processEngineGenStatus2(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processEngineGenStatus3(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processEngineGenStatus5(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processFuelSystemRequest2(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processEngineGenStatus4(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processTransmissionStatus3(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processTransmissionStatus2(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processPlatformConfiguration(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processTransOutRotation(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	void processVehicleSpeed(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	 
};


