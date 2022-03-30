//
//  CanBusMgr.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//
#pragma once


#include <unistd.h>
#include <sys/time.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <thread>			//Needed for std::thread

#include <unistd.h>
#include <sys/time.h>

#include "CommonDefs.hpp"
#include "FrameDB.hpp"
 
using namespace std;
 
class CANBusMgr {
	
public:

 
	static CANBusMgr *shared() {
		if(!sharedInstance){
			sharedInstance = new CANBusMgr;
		}
		return sharedInstance;
	}

	CANBusMgr();
  ~CANBusMgr();
 	
	bool registerHandler(string ifName);
	void unRegisterHandler(string ifName);

	bool start(string ifName,int *error = NULL);
	bool stop(string ifName,int *error = NULL);

	bool readFramesFromFile(string filePath, int *error = NULL);
 
private:
	
	void run();

	std::thread  	_thread;		 //Internal thread, this is in order to start and stop the thread from
	bool 				_running;	 //Flag for starting and terminating the main loop

	int openSocket(string ifName, int *error = NULL);

	map<string, int> _interfaces;
	fd_set	 _master_fds;		//Socket descriptor set that holds the sockets that are ready for read
	int		_max_fds;
	
	static CANBusMgr *sharedInstance;
};

