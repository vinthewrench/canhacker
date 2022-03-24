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
 
#include "CanProtocol.hpp"

using namespace std;

struct  frame_entry{
	can_frame_t 	frame;
	uint				line;			// line number for this frame
	long				timeStamp;
	long				avgTime;		 // how often do we see these  ((now - lastTime) + avgTime) / 2
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
  
  void saveFrame(string ifName, CanProtocol *protocol, can_frame_t frame, long timeStamp);
  void clearFrames(string ifName);
  
	void dumpFrames();

  
private:
	
	static FrameMgr *sharedInstance;
	map <string , map<canid_t,frame_entry>>  frameMap;
};

