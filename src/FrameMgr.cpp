//
//  FrameMgr.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//

#include "FrameMgr.hpp"
#include <bitset>


inline  std::string hexDumpFrame(can_frame_t frame) {
	
	char      	 lineBuf[80];
	char       	 *p = lineBuf;

	p += sprintf(p, "%03X [%d] ",  frame.can_id, frame.can_dlc);
	
	for (int i = 0; i < frame.can_dlc; i++) p += sprintf(p,"%02X ",frame.data[i]);
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p,"   ");
	p += sprintf(p,"  ");
	for (int i = 0; i < frame.can_dlc; i++)  {
		
		uint8_t c = frame.data[i] & 0xFF;
		if (c > ' ' && c < '~')
			*p++ = c ;
		else {
			*p++ = '.';
		}
	}
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p," ");

	*p++ = 0;
	return string(lineBuf);
}

FrameMgr *FrameMgr::sharedInstance = NULL;


FrameMgr::FrameMgr(){
	frameMap.clear();
}
FrameMgr::~FrameMgr(){
	
}
 
void FrameMgr::clearFrames(string ifName){
	if(ifName.empty()){
		frameMap.clear();
	}else {
		if(frameMap.count(ifName)){
			frameMap[ifName].clear();
		}
	}
 }

 
void  FrameMgr::saveFrame(string ifName, CanProtocol *protocol, can_frame_t frame, long timeStamp){
	
 	bitset<8> changed;
	bool isNew = false;
	
	canid_t can_id = frame.can_id & CAN_ERR_MASK;
	
 	// is it new  interface  then create it
	if(frameMap.count(ifName) == 0){
		frameMap[ifName] = {};
	}
	// get the map entry for that interface.
	auto m1 = frameMap.find(ifName);
	auto theFrames = &m1->second;
	
	size_t count =  theFrames->count(can_id);
	if( count == 0){
		// create new frame entry
			frame_entry entry;
			entry.frame = frame;
			entry.line = (uint) theFrames->size();
			entry.timeStamp = timeStamp;
			entry.avgTime = 0;
			theFrames->insert( std::pair<canid_t,frame_entry>(can_id,entry));
			isNew = true;
	}
	else {
		// can ID is already there
		auto e = theFrames->find(can_id);
		auto oldFrame = &e->second.frame;
		
		if(frame.can_dlc == oldFrame->can_dlc
			&& memcmp(frame.data, oldFrame->data, frame.can_dlc ) == 0){
			// frames are same - update timestamp and average
			long newAvg = ((timeStamp - e->second.timeStamp) + e->second.avgTime) / 2;
			e->second.timeStamp = timeStamp;
			e->second.avgTime = newAvg;
		}
		else {
			// it changed
			//did the length change?
			if(frame.can_dlc != oldFrame->can_dlc) {
				for(int i = 0; i < frame.can_dlc; i++)
					changed.set(i);
			}
			else {
				for(int i = 0; i < frame.can_dlc; i++) {
					if(frame.data != oldFrame->data)
						changed.set(i);
				}
			}
			
			// either way - update the frame
			// copy the frame
			memcpy( oldFrame,  &frame, sizeof(can_frame_t) );
	
			//- update timestamp and average
			long newAvg = ((timeStamp - e->second.timeStamp) + e->second.avgTime) / 2;
			e->second.timeStamp = timeStamp;
			e->second.avgTime = newAvg;

 		}
	}
 
	
	
//	if(updated){
// 		printf("%5ld %s %s\r\n", timeStamp,
//				 		hexDumpFrame(frame).c_str(),
//						 protocol?protocol->descriptionForFrame(frame).c_str():"");
				
//	}
// 
}

static bool cmpFrameLines(pair<canid_t,frame_entry> &a, pair<canid_t,frame_entry> &b ){
	return a.second.line < b.second.line;
}


void FrameMgr::dumpFrames(){
	for (const auto& [ifName, frames] : frameMap) {
		printf("Interface %s\n", ifName.c_str());
		
		vector<pair<canid_t, frame_entry> > sortedFrames;
		for (auto& it : frames)
			sortedFrames.push_back(it);
		 
		// Sort using comparator function
		 sort(sortedFrames.begin(), sortedFrames.end(), cmpFrameLines);
 
			for (const auto& [frameID, frame] : sortedFrames) {
				 printf("%2d %05ld  %s \r\n",
						  frame.line,
						  frame.avgTime,
							hexDumpFrame(frame.frame).c_str()
						  );
			}
	}
}


