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
#include <cassert>

#include "GMLAN.hpp"
#include  "Wranger2010.hpp"

/* add a fd to fd_set, and update max_fd */
static int safe_fd_set(int fd, fd_set* fds, int* max_fd) {
	 assert(max_fd != NULL);

	 FD_SET(fd, fds);
	 if (fd > *max_fd) {
		  *max_fd = fd;
	 }
	 return 0;
}

/* clear fd from fds, update max fd if needed */
static int safe_fd_clr(int fd, fd_set* fds, int* max_fd) {
	 assert(max_fd != NULL);

	 FD_CLR(fd, fds);
	 if (fd == *max_fd) {
		  (*max_fd)--;
	 }
	 return 0;
}

CANBusMgr *CANBusMgr::sharedInstance = NULL;

CANBusMgr::CANBusMgr(){
	_interfaces.clear();
	_running = true;
	FD_ZERO(&_master_fds);
	_max_fds = 0;
	
	_thread = std::thread(&CANBusMgr::run, this);

}

CANBusMgr::~CANBusMgr(){
	
	stop("");
	
	FD_ZERO(&_master_fds);
	_max_fds = 0;
	
	_running = false;
	 
 if (_thread.joinable())
		_thread.join();

}

bool CANBusMgr::registerHandler(string ifName) {
	
	// is it an already registered ?
 	if(_interfaces.count(ifName))
		return false;
	
	 	_interfaces[ifName] = -1;
	
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
	
	for (auto& [key, fd]  : _interfaces){
		if (strcasecmp(key.c_str(), ifName.c_str()) == 0){
 			if(fd == -1){
				// open connection here
				fd = openSocket(ifName, errorOut);
				_interfaces[ifName] = fd;
 				return fd == -1?false:true;
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
	
	// add to read set
	safe_fd_set(fd, &_master_fds, &_max_fds);
	
	return fd;
}


bool CANBusMgr::stop(string ifName, int *errorOut){
	
	// close all?
	if(ifName.empty()){
		for (auto& [key, fd]  : _interfaces){
			if(fd != -1){
				close(fd);
				safe_fd_clr(fd, &_master_fds, &_max_fds);
				_interfaces[ifName] = -1;
			}
		}
		return true;
 	}
	else for (auto& [key, fd]  : _interfaces){
		if (strcasecmp(key.c_str(), ifName.c_str()) == 0){
			if(fd != -1){
				close(fd);
				safe_fd_clr(fd, &_master_fds, &_max_fds);
				_interfaces[ifName] = -1;
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
 
	frameDB->clearFrames();
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
			
			frame.can_dlc = 0;
			while(*p) {
				uint8_t b1;
		
				if(sscanf(p, "%02hhx", &b1) != 1){
					failed = true;
					break;
				}
				
				frame.data[frame.can_dlc++] = b1;
				p+=2;
				if(frame.can_dlc > CAN_MAX_DLEN) {
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
	}
	catch(std::ifstream::failure &err) {
		
		printf("readFramesFromFile FAIL: %s", err.what());
		statusOk = false;
	}
	
	return statusOk;
}



void CANBusMgr::run() {
	
	FrameDB* frameDB = FrameDB::shared();
	struct can_frame frame;

	while(_running){
		
		/* wait for something to happen on the socket */
		struct timeval selTimeout;
		selTimeout.tv_sec = 0;       /* timeout (secs.) */
		selTimeout.tv_usec = 100;            /* 100 microseconds */
		
		/* back up master */
		fd_set dup = _master_fds;
		
		int numReady = select(_max_fds+1, &dup, NULL, NULL, &selTimeout);
		if( numReady == -1 ) {
			perror("select");
			_running = false;
		}
		
		/* check which fd is avaialbe for read */
		for (auto& [ifName, fd]  : _interfaces) {
			if ((fd != -1)  && FD_ISSET(fd, &dup)) {
				time_t now =  time(NULL);
				
				size_t nbytes = read(fd, &frame, sizeof(struct can_frame));
				
				if(nbytes == 0){ // shutdown
					_interfaces[ifName] = -1;
				}
				else if(nbytes > 0){
					frameDB->saveFrame(ifName, frame, now);
				}
			}
		}
	}
}
