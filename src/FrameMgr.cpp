//
//  FrameMgr.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//

#include "FrameMgr.hpp"

//static inline  std::string hexDumpFrame(can_frame_t frame, bitset<8> changed) {
//	
//	char      	 lineBuf[120];
//	char       	 *p = lineBuf;
//
//	p += sprintf(p, "%03X [%d] ",  frame.can_id, frame.can_dlc);
//	
//	for (int i = 0; i < frame.can_dlc; i++)
//			p += sprintf(p,"%s%02X\x1b[0m ",
//							 changed.test(i)?"\x1b[91m":"",
//							 frame.data[i]);
//	 
//	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p,"   ");
//	p += sprintf(p,"  ");
//	for (int i = 0; i < frame.can_dlc; i++)  {
//		
//		uint8_t c = frame.data[i] & 0xFF;
//		if (c > ' ' && c < '~')
//			*p++ = c ;
//		else {
//			*p++ = '.';
//		}
//	}
//	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p," ");
//
//	*p++ = 0;
//	return string(lineBuf);
//}

FrameMgr *FrameMgr::sharedInstance = NULL;


FrameMgr::FrameMgr(){
	_interfaces.clear();
	_eTag = 0;
}

FrameMgr::~FrameMgr(){
	
}
 

bool FrameMgr::registerHandler(string ifName, CanProtocol *protocol) {
	
	// is it an already registered ?
	if(_interfaces.count(ifName))
		return false;
	
	interfaceInfo_t ifInfo;
	ifInfo.ifName = ifName;
	ifInfo.protocol = protocol;
	ifInfo.frames.clear();
	
	_interfaces[ifName] = ifInfo;
	
	return true;
}

void FrameMgr::unRegisterHandler(string ifName){
	
	if(_interfaces.count(ifName)){
//		auto ifInfo = _interfaces[ifName];
	 _interfaces.erase(ifName);
	}
}


CanProtocol* FrameMgr::protocolForHandler(string ifName){
	
	CanProtocol* proto = NULL;
	
	if(_interfaces.count(ifName)){
		// get the map entry for that interface.
		auto m1 = _interfaces.find(ifName);
		proto = m1->second.protocol;

	}
	return proto;
};


void FrameMgr::clearFrames(string ifName){
	
	// close all?
	if(ifName.empty()){
		for (auto& [key, entry]  : _interfaces){
			entry.frames.clear();
		}
	}
	else for (auto& [key, entry]  : _interfaces){
		if (strcasecmp(key.c_str(), ifName.c_str()) == 0){
			entry.frames.clear();
			return;
		}
	}
}

 
void  FrameMgr::saveFrame(string ifName, can_frame_t frame, long timeStamp){
	
	std::lock_guard<std::mutex> lock(_mutex);

	bitset<8> changed;
	bool isNew = false;
	uint line = 0;
	long avgTime = 0;
	
	canid_t can_id = frame.can_id & CAN_ERR_MASK;

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
		throw CanMgrException("ifName name is not registered");
	}
	
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;
	
	size_t count =  theFrames->count(can_id);
	if( count == 0){
		// create new frame entry
		frame_entry entry;
		entry.frame = frame;
		entry.line = (uint) theFrames->size();
		entry.timeStamp = timeStamp;
		entry.avgTime = 0;
		entry.eTag = _eTag++;
		entry.updateTime = time(NULL);
		entry.lastChange.reset();
		theFrames->insert( std::pair<canid_t,frame_entry>(can_id,entry));
		isNew = true;
		line = entry.line;
	}
	else {
		// can ID is already there
		auto e = theFrames->find(can_id);
		auto oldFrame = &e->second.frame;
		
		line = e->second.line;
		
		if(frame.can_dlc == oldFrame->can_dlc
			&& memcmp(frame.data, oldFrame->data, frame.can_dlc ) == 0){
			// frames are same - update timestamp and average
			long newAvg = ((timeStamp - e->second.timeStamp) + e->second.avgTime) / 2;
			e->second.timeStamp = timeStamp;
			e->second.avgTime = newAvg;
			avgTime =  newAvg;
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
			e->second.eTag = _eTag++;
			e->second.updateTime = time(NULL);
			e->second.lastChange = changed;
			avgTime =  newAvg;
		}
	}
	
//	if(isNew || changed.count() > 0) {
//		printf("\x1b[%d;0H%6ld %s  ",
//				 line + 4,
//				 avgTime,
//				 hexDumpFrame(frame,changed).c_str());
//	//			 protocol?protocol->descriptionForFrame(frame).c_str():"");
//
//	}
	
}


vector<canid_t> FrameMgr::framesUpdateSinceEtag(string ifName, eTag_t eTag, eTag_t *eTagOut ){
	vector<canid_t> can_ids;
	
	std::lock_guard<std::mutex> lock(_mutex);

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
		throw CanMgrException("ifName name is not registered");
	}
	
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;

	for (const auto& [canid, frame] : *theFrames) {
		if(frame.eTag < eTag)
			can_ids.push_back(canid);
	}
	
	if(eTagOut)
		*eTagOut = _eTag;
	
	return can_ids;
}

vector<canid_t>  	FrameMgr::framesOlderthan(string ifName, time_t time){
	vector<canid_t> can_ids;
	
	std::lock_guard<std::mutex> lock(_mutex);

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
		throw CanMgrException("ifName name is not registered");
	}
	
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;

	for (const auto& [canid, frame] : *theFrames) {
		if(frame.updateTime < time)
			can_ids.push_back(canid);
	}
	
	return can_ids;
}

bool FrameMgr::frameWithCanID(string ifName, canid_t can_id, frame_entry *frameOut){
	
	frame_entry entry;
		
	std::lock_guard<std::mutex> lock(_mutex);

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
		throw CanMgrException("ifName name is not registered");
	}
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;

	if(theFrames->count(can_id) == 0 )
		return false;
	
	if(frameOut){
		auto e = theFrames->find(can_id);
		*frameOut =  e->second;
	}
 
	return true;
	
}

//
//static bool cmpFrameLines(pair<canid_t,frame_entry> &a, pair<canid_t,frame_entry> &b ){
//	return a.second.line < b.second.line;
//}
//
//
////void FrameMgr::dumpFrames(){
////	for (const auto& [ifName, frames] : frameMap) {
////		printf("Interface %s\n", ifName.c_str());
////
////		vector<pair<canid_t, frame_entry> > sortedFrames;
////		for (auto& it : frames)
////			sortedFrames.push_back(it);
////
////		// Sort using comparator function
////		 sort(sortedFrames.begin(), sortedFrames.end(), cmpFrameLines);
////
////			for (const auto& [frameID, frame] : sortedFrames) {
////				 printf("%2d %05ld  %s \r\n",
////						  frame.line,
////						  frame.avgTime,
////							hexDumpFrame(frame.frame).c_str()
////						  );
////			}
////	}
//}
//
//
