//
//  Wranger2010.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//
 

#include "CanProtocol.hpp"
#pragma once
 
class Wranger2010: public CanProtocol {
	
public:
	
	typedef  enum  {
		STEERING_ANGLE,
		MILEAGE,
		KEY_POSITION,
		FUEL_LEVEL,
		DOORS,
		CLOCK,
		
	 
		/// Not a valid value; this is the number of members in this enum
		count,
		// helpers for iterating over the enum
		begin = 0,
		end = count,
	} value_keys_t;

	Wranger2010();

	virtual void registerSchema(FrameDB*);

	virtual void processFrame(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	virtual string nameForFrame(can_frame_t frame);
	virtual string descriptionForFrame(can_frame_t frame);
 
	
	map<value_keys_t,  string_view> _value_key_map = {
		{STEERING_ANGLE , 		"JK_STEERING_ANGLE"},
		{MILEAGE , 					"JK_MILEAGE"},
		{KEY_POSITION, 			"JK_KEY_POSITION"},
		{FUEL_LEVEL, 				"JK_FUEL_LEVEL"},
		{DOORS, 						"JK_DOORS"},
		{CLOCK, 						"JK_CLOCK"},
	};
	

private:
	
};


