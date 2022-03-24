//
//  FrameMgr.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//

#pragma once

#include "CommonDefs.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <bitset>
 
#include "CanProtocol.hpp"

using namespace std;

struct  frame_entry{
	can_frame_t 	frame;
	uint				line;			// line number for this frame
	long				timeStamp;	// from canbus (tv.tv_sec - start_tv.tv_sec) * 100 + (tv.tv_usec / 10000);
	long				avgTime;		 // how often do we see these  ((now - lastTime) + avgTime) / 2
	eTag_t 			eTag;
	time_t			updateTime;
	bitset<8> 		lastChange;
};

class FrameMgr {
	
public:
	
	static FrameMgr *shared() {
		if(!sharedInstance){
			sharedInstance = new FrameMgr;
		}
		return sharedInstance;
	}
	
	
	FrameMgr();
	~FrameMgr();
	
	bool registerHandler(string ifName,  CanProtocol *protocol = NULL);
	void unRegisterHandler(string ifName);
	CanProtocol*		protocolForHandler(string ifName);
	
	void saveFrame(string ifName, can_frame_t frame, long timeStamp);
	void clearFrames(string ifName);
		
	eTag_t lastEtag() { return  _eTag;};
	
	vector<canid_t>  	framesUpdateSinceEtag(string ifName, eTag_t eTag, eTag_t *newEtag);
	vector<canid_t>  	framesOlderthan(string ifName, time_t time);
	bool 					frameWithCanID(string ifName, canid_t can_id, frame_entry *frame);
	
private:
	
	static FrameMgr *sharedInstance;
	
	mutable std::mutex _mutex;
	eTag_t _eTag;
	
	typedef struct {
		string							ifName;
		CanProtocol*   				protocol;
		map<canid_t,frame_entry> 	frames;
	} interfaceInfo_t;

	map<string, interfaceInfo_t> _interfaces;

};



