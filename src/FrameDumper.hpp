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
	typedef  pair<string, canid_t> filter_t;
	
	typedef  enum  {
		 NONE,
		 VALUES,
		 FRAMES,
		 BOTH,
		 } dump_mode_t;

	FrameDumper();
	~FrameDumper();
	
	void start(string ifName = string());
	void stop();
	void pause();
	void resume();

	void setDumpMode(dump_mode_t mode);
	dump_mode_t getDumpMode(){ return _mode;};

	void setFilters(vector<filter_t> canIds);
	vector<filter_t> getFilters() {return _filters;};


private:
	void run();

	void printChangedFrames(string ifName, bool redraw = false);
	void printChangedValues(int lastLine, bool redraw = false);
	void printHeaderLine();
	
	std::thread  	_thread;		 //Internal thread, this is in order to start and stop the thread from
	bool 				_running;	 //Flag for starting and terminating the main loop
	bool 				_paused;

	bool 				_didStop;		// when loop is done
	string 			_ifName;

	mutable std::mutex _mutex;

	dump_mode_t		_mode;
	dump_mode_t 	_lastmode;
	
	vector< filter_t> _filters;		//  {can0,0x219}
	
	unsigned long 	_firstTimeStamp;
	unsigned long 	_lastTimeStamp;
	eTag_t 			_lastEtag;
	eTag_t 			_lastValueEtag;

	int 				_topOffset;
	
	map<frameTag_t, int> _frameLineMap;
	int _lastFrameLine;

	map<string_view, int> _valueLineMap;
	int _lastValueLine;

	
};
