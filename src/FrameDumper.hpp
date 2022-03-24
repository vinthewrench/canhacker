//
//  FrameDumper.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/24/22.
//
#pragma once

#include "CommonDefs.hpp"

#include "FrameMgr.hpp"

#include <string>

 #include <thread>			//Needed for std::thread

using namespace std;

class FrameDumper {
	
public:

	FrameDumper();
	~FrameDumper();
	
	void start(string ifName);
	void stop();

private:
	void run();

	std::thread  	_thread;		 //Internal thread, this is in order to start and stop the thread from
	bool 				_running;	 //Flag for starting and terminating the main loop
	string 			_ifName;
	
	eTag_t 			_lastEtag;

};
