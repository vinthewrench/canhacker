//
//  Wranger2010.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//
 

#include "CanProtocol.hpp"
#include "FrameDB.hpp"

#pragma once
 
class Wranger2010: public CanProtocol {
	
public:
 
	Wranger2010();

	virtual void registerSchema(FrameDB*);
	virtual void reset();
	virtual void processFrame(FrameDB* db,string ifName, can_frame_t frame, time_t when);

	virtual string descriptionForFrame(can_frame_t frame);
 
 
private:
	
	 void processPrivateFrame(FrameDB* db,string ifName, can_frame_t frame, time_t when);

	void  processPrivateODB(FrameDB* db,time_t when,
											canid_t can_id,
										  	bool isRequest,
											uint8_t service_id,
										 	uint16_t len, uint8_t* data);
	

	string_view schemaKeyForValueKey(int valueKey);
	
	
	static void processRadioFrameWrapper(void* context,
													 string ifName, canid_t can_id,
													can_frame_t frame, unsigned long timeStamp);
	void processRadioFrame(string ifName, canid_t can_id, can_frame_t frame, unsigned long timeStamp);


	FrameDB* _db;
	
	string _VIN;
	
	// similar to ODB but private
	typedef struct {
		canid_t			can_id;
		uint8_t			service_id;
		uint8_t			pid;
		uint8_t			rollingcnt; 	// next expected cnt
		uint16_t			total_len;
		uint16_t			current_len;

		uint8_t			buffer[4096];
		} obd_state_t;

	map<canid_t,obd_state_t> _multi_frame;

};


