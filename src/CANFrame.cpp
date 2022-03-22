//
//  CANFrame.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//

#include "CANFrame.hpp"
#include <string.h>

CANFrame::CANFrame(){
	frames.clear();
}
CANFrame::~CANFrame(){
	
}


void  CANFrame::saveFrame(can_frame_t frame, long timeStamp){
	
 
	bool updated = false;
  
	// is it an existing frame?
	if(frames.count(frame.can_id)) {
		auto oldFrame = frames[frame.can_id];
		
		if(frame.can_dlc == oldFrame.len
			&& memcmp(frame.data, oldFrame.data, frame.can_dlc ) == 0){
	 
			long newAvg = ((timeStamp - oldFrame.timeStamp) + oldFrame.avgTime) / 2;
			
			oldFrame.timeStamp = timeStamp;
			oldFrame.avgTime = newAvg;
		}
	}
	else { // it changed
		timed_can_frame newframe;
		bzero((void*) &newframe, sizeof(timed_can_frame));
		
		newframe.can_id = frame.can_id;
		newframe.timeStamp = timeStamp;
		newframe.avgTime = 0;
		memcpy(newframe.data, frame.data, frame.can_dlc);
		newframe.len = frame.can_dlc;
		frames[frame.can_id] = newframe;
		updated = false;
	}
	 
	if(updated){
		printf("%s\n",hexDumpFrame(frame).c_str());
	}
 
}



void CANFrame::clearFrames(){
	frames.clear();
}


