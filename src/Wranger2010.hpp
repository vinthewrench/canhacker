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

	virtual void processFrame(FrameDB* db, can_frame_t frame, time_t when, eTag_t eTag);

	virtual string descriptionForFrame(can_frame_t frame);
 
 
private:
	string_view schemaKeyForValueKey(int valueKey);

};

