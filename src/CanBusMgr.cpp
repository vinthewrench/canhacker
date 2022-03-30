//
//  CanBusMgr.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//

#include "CanBusMgr.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

#include "GMLAN.hpp"
#include  "Wranger2010.hpp"

CANBusMgr *CANBusMgr::sharedInstance = NULL;

CANBusMgr::CANBusMgr(){
	_interfaces.clear();
	_running = true;
	
	_thread = std::thread(&CANBusMgr::run, this);

}

CANBusMgr::~CANBusMgr(){
	
	stop("");
	
	_running = false;
	 
 if (_thread.joinable())
		_thread.join();

}

bool CANBusMgr::registerHandler(string ifName) {
	
	// is it an already registered ?
 	if(_interfaces.count(ifName))
		return false;
	
	interfaceInfo_t ifInfo;
	ifInfo.fd = -1;
	ifInfo.ifName = ifName;
 
	_interfaces[ifName] = ifInfo;
	
	return true;
}

void CANBusMgr::unRegisterHandler(string ifName){
	
	if(_interfaces.count(ifName)){
//		auto ifInfo = _interfaces[ifName];
		stop(ifName);
		_interfaces.erase(ifName);
	}
	
}

bool CANBusMgr::start(string ifName,int *errorOut){
	
	for (auto& [key, entry]  : _interfaces){
		if (strcasecmp(key.c_str(), ifName.c_str()) == 0){
			if(entry.fd == -1){
				// open connection here
				entry.fd = openSocket(ifName, errorOut);
 				return entry.fd == -1?false:true;
			}
		}
	}
	
	if(errorOut) *errorOut = ENXIO;
	return false;
}

int CANBusMgr::openSocket(string ifname, int *errorOut){
	int fd = -1;
	
	// create a socket
	fd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(fd == -1){
		if(errorOut) *errorOut = errno;
		return -1;
	}
	
	// Get the index number of the network interface
	unsigned int ifindex = if_nametoindex(ifname.c_str());
	if (ifindex == 0) {
		if(errorOut) *errorOut = errno;
		return -1;
	}

	// fill out Interface request structure
	struct sockaddr_can addr;
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifindex;

	if (::bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if(errorOut) *errorOut = errno;
		return -1;
	}
 
	return fd;
}


bool CANBusMgr::stop(string ifName, int *errorOut){
	
	// close all?
	if(ifName.empty()){
		for (auto& [key, entry]  : _interfaces){
			if(entry.fd != -1){
				close(entry.fd);
				entry.fd = -1;
			}
		}
		return true;
 	}
	else for (auto& [key, entry]  : _interfaces){
		if (strcasecmp(key.c_str(), ifName.c_str()) == 0){
			if(entry.fd != -1){
				close(entry.fd);
				entry.fd = -1;
				}
			return true;
		}
	}
	if(errorOut) *errorOut = ENXIO;
	return false;
}

bool CANBusMgr::readFramesFromFile(string filePath, int *errorOut){
	
	
	std::ifstream	ifs;
	bool 				statusOk = false;
	
	struct timeval start_tv = {0,0};
	
	FrameDB* frameDB = FrameDB::shared();
 
	frameDB->clearFrames("");
	if(filePath.empty())
			return false;
 
	uint32_t number = 0;
	try{
		string line;
	
		// open the file
		ifs.open(filePath, ios::in);
		if(!ifs.is_open()) {
			if(errorOut) *errorOut = errno;
			return false;
		}
		
		while ( std::getline(ifs, line) ) {
			
			number++;
			
			bool failed = false;
			
			struct can_frame frame;
			struct timeval tv;
			long timestamp = 0;
			
//			size_t nbytes = read(_fd, &frame, sizeof(struct can_frame));
//
//			if(nbytes >= sizeof(struct can_frame)){
//
//				struct timeval tv;
//				gettimeofday(&tv, NULL);
//				long timestamp = (tv.tv_sec - start_tv.tv_sec) * 100 + (tv.tv_usec / 10000);
//				rcvFrame(frame, timestamp);
//
//			timed_frame frame;
//
			const char *p = line.c_str() ;
			int n;
			char canport[20];
			
			if( sscanf(p,"(%ld.%d) %s%x#%n",
						  &tv.tv_sec,
						  &tv.tv_usec,
						  canport,
						  &frame.can_id,
						  &n) != 4) continue;
			
			if(number == 1){
				start_tv = tv;
			}
			
			timestamp = (tv.tv_sec - start_tv.tv_sec) * 100 + (tv.tv_usec / 10000);

			p = p+n;
			
			frame.len = 0;
			while(*p) {
				uint8_t b1;
		
				if(sscanf(p, "%02hhx", &b1) != 1){
					failed = true;
					break;
				}
				
				frame.data[frame.len++] = b1;
				p+=2;
				if(frame.len > CAN_MAX_DLEN) {
					failed = true;
					break;
				}
			}
			if(!failed){
				frameDB->saveFrame(string(canport), frame, timestamp);
				usleep(500);
			}
		}
 
		statusOk = true;
		ifs.close();
		
		usleep(500);

		
	//	frameDB->dumpFrames();
	}
	catch(std::ifstream::failure &err) {
		
		printf("readFramesFromFile FAIL: %s", err.what());
		statusOk = false;
	}
	
	return statusOk;
}


void CANBusMgr::run() {
	
	FrameDB* frameDB = FrameDB::shared();
	
	while(_running){

		usleep(100);
	}
}
