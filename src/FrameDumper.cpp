//
//  FrameDumper.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/24/22.
//

#include "FrameDumper.hpp"
#include <unistd.h>
#include <sys/time.h>


static inline  std::string hexDumpFrame(can_frame_t frame, bool isOld, bitset<8> changed) {
	
	char      	 lineBuf[120];
	char       	 *p = lineBuf;

	p += sprintf(p, "%03X [%d] ",  frame.can_id, frame.can_dlc);
	
	for (int i = 0; i < frame.can_dlc; i++){
			const char* useColor = "";
		
			if(isOld) useColor = "\x1b[33m";
			else if (changed.test(i))  useColor = "\x1b[91m";
	
			p += sprintf(p,"%s%02X\x1b[0m ",  useColor, frame.data[i]);

	}
		
		 
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p,"   ");
	p += sprintf(p,"  ");
	
	if(isOld) p += sprintf(p,"\x1b[33m");
	
	for (int i = 0; i < frame.can_dlc; i++)  {
		
		uint8_t c = frame.data[i] & 0xFF;
		if (c > ' ' && c < '~')
			*p++ = c ;
		else {
			*p++ = '.';
		}
	}
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p," ");
	p += sprintf(p,"\x1b[0m");
	
	*p++ = 0;
	return string(lineBuf);
}


FrameDumper::FrameDumper(){
	_ifName = "";
	_running = true;
	_thread = std::thread(&FrameDumper::run, this);

}
FrameDumper::~FrameDumper(){
	stop();
	
	_running = false;
	 
 if (_thread.joinable())
		_thread.join();

}

void FrameDumper::start(string ifName){
	
	
	// clear screen
	printf("\x1b[2J\x1b[0;0H\x1b[?25l");

	_lastEtag = 0;
	_ifName = ifName;
	
}

void FrameDumper::stop(){
	_ifName = "";
	printf("\x1b[70;0H\x1b[?25h");
}



void FrameDumper::run() {
	
	FrameMgr* frameMgr = FrameMgr::shared();
	
	while(_running){

	// cache this each time , it could change async
		string ifName = _ifName;

		if(!ifName.empty()){
			time_t now = time(NULL);
			
			CanProtocol* proto = frameMgr->protocolForHandler(ifName);
	 
			auto  can_ids = frameMgr->framesUpdateSinceEtag(ifName, _lastEtag, &_lastEtag);
			auto old_ids = frameMgr->framesOlderthan(ifName, now - 5);
	 
			for(auto can_id : can_ids){
				frame_entry frame;
				bool isOldFrame = find(old_ids.begin(), old_ids.end(), can_id) != old_ids.end();
				if( frameMgr->frameWithCanID(ifName, can_id, &frame)){
				
					printf("\x1b[%d;0H%6ld %s",
							 frame.line +4,
							 frame.avgTime,
							 hexDumpFrame(frame.frame, isOldFrame, frame.lastChange).c_str());
					if(proto)
						printf("   %s", proto->descriptionForFrame(frame.frame).c_str());
				}
			}
		}
		
		usleep(500);
	}
}
