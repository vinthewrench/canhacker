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
#include <fstream>

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

	bool readFramesFromFile(string filePath, int *error = NULL,  voidCallback_t doneCallBack = NULL );
	void quitReading() {_reading = false;};
	bool isReadingFile(){ return _reading;};
	
	bool sendFrame(string ifName, canid_t can_id, vector<uint8_t> bytes,  int *error = NULL);
	
private:
	

	void readFileThread(std::ifstream *ifs, voidCallback_t doneCallBack);
	std::thread  	_thread;		 //CANbus reading  thread,
	bool 				_running;	 //Flag for starting and terminating the main loop
 
	
	void CANThread();
	std::thread  	_thread1;	//thread,for reading a file
	bool 				_reading;	// reading frames from file
	
	int openSocket(string ifName, int *error = NULL);

	map<string, int> _interfaces;
	fd_set	 _master_fds;		// Can sockets that are ready for read
	int		_max_fds;
	
	static CANBusMgr *sharedInstance;
};

