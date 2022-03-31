//
//  FrameDumper.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/24/22.
//
#pragma once

#include "CommonDefs.hpp"
#include "FrameDB.hpp"

#include <string>

 #include <thread>			//Needed for std::thread

using namespace std;

class FrameDumper {
	
public:

	FrameDumper();
	~FrameDumper();
	
	void start(string ifName = string());
	void stop();

 
private:
	void run();

	void printChangedFrames(string ifName, bool redraw = false);
	void printChangedValues(int lastLine, bool redraw = false);
	void printHeaderLine();
	
	std::thread  	_thread;		 //Internal thread, this is in order to start and stop the thread from
	bool 				_running;	 //Flag for starting and terminating the main loop
	bool 				_didStop;		// when loop is done
	string 			_ifName;

	
	bool				_showFrames;
	bool 				_showValues;
	
	eTag_t 			_lastEtag;

	int 				_topOffset;
	
	map<frameTag_t, int> _frameLineMap;
	int _lastFrameLine;

	map<string_view, int> _valueLineMap;
	int _lastValueLine;

	
};
