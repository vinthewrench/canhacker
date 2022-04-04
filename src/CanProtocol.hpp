//
//  CanProtocol.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//

#pragma once

#include "CommonDefs.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <bitset>
#include <cstdint>

typedef struct can_frame can_frame_t;

using namespace std;

class FrameDB;

class CanProtocol {

public:
 
	virtual void registerSchema(FrameDB*) {};
	
	virtual void reset()  {};
	virtual void processFrame(FrameDB* db, string ifName,  can_frame_t frame, time_t when, eTag_t eTag){};
	virtual string descriptionForFrame(can_frame_t frame)  {return "";};
 
 
};

