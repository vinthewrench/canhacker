//
//  CANbus.cpp
//  canrecord
//
//  Created by Vincent Moscaritolo on 3/18/22.
//

#include "CANbus.hpp"
#include <iostream>
#include <sstream>
#include <fstream>


CANbus::CANbus(){
	_fd = -1;
	_isSetup = false;
	_running = false;
}

CANbus::~CANbus(){
	
	_fd = -1;
	_isSetup = false;

}


bool CANbus::begin(const char *ifname,  int * errorOut){
 	
	struct sockaddr_can addr;
	
	// create a socket
	_fd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(_fd == -1){
		if(errorOut) *errorOut = errno;
		return false;
	}
	
	// Get the index number of the network interface	
	unsigned int ifindex = if_nametoindex(ifname);

	if (ifindex == 0) {
		if(errorOut) *errorOut = errno;
		return false;
	}

	// fill out Interface request structure
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifindex;

	if (::bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if(errorOut) *errorOut = errno;
		return false;
	}
	
	_isSetup = true;
	_running = true;

	_thread = std::thread(&CANbus::run, this);

	return true;
};


bool CANbus::setFilter(canid_t can_id, canid_t can_mask,int * errorOut) {
	
	if(!_isSetup) return false;

	 struct can_filter filter;
	 filter.can_id = can_id;
	 filter.can_mask = can_mask;

	if (::setsockopt(_fd, SOL_CAN_RAW, CAN_RAW_FILTER,
						  &filter, sizeof(struct can_filter)) < 0) {
		if(errorOut) *errorOut = errno;
		return false;
	}
	
	return true;
}

bool CANbus::sendFrame(canid_t frameID,  uint8_t* data, size_t dataLen) {
	if(!_isSetup) return false;

	if(dataLen > 8) return false;
	
	struct can_frame frame;
	memset(&frame, 0, sizeof(frame));

	frame.can_id = frameID;
	frame.can_dlc = dataLen;
	memcpy(frame.data, data, dataLen);
	
	if (write(_fd, &frame,
				 sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		return false;
	}
	
	return true;
}

void CANbus::stop(){
	
	_running = false;
	 
 if (_thread.joinable())
		_thread.join();
	
	if(_fd > -1){
		close(_fd);
	}
	_fd = -1;
	_isSetup = false;
}

void CANbus::run() {

	struct timeval  start_tv;
	
	gettimeofday(&start_tv, NULL);

	while(_running){

		/* wait for something to happen on the socket */
		struct timeval selTimeout;
		selTimeout.tv_sec = 0;       /* timeout (secs.) */
		selTimeout.tv_usec = 100;            /* 100 microseconds */
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(_fd, &readSet);
 
		int numReady = select(_fd+1, &readSet, NULL, NULL, &selTimeout);
		if( numReady == -1 ) {
			perror("select");
			_running = false;
		}
		if (FD_ISSET(_fd, &readSet)){
			struct can_frame frame;

			size_t nbytes = read(_fd, &frame, sizeof(struct can_frame));

			if(nbytes >= sizeof(struct can_frame)){

 				struct timeval tv;
				gettimeofday(&tv, NULL);
				long timestamp = (tv.tv_sec - start_tv.tv_sec) * 100 + (tv.tv_usec / 10000);
	//			rcvFrame(frame, timestamp);
			}
		 
			if(nbytes == 0){ // shutdown
				_running = false;
			}
		}
	}

	
	// Close all sockets
	if(_fd > -1){
		close(_fd);
	}
	_fd = -1;

}

bool CANbus::readFramesFromFile(string filePath, int * errorOut){
	std::ifstream	ifs;
	bool 				statusOk = false;
	
 
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
	//			rcvFrame(frame, timestamp);
			}
 		}
 
		statusOk = true;
		ifs.close();
	}
	catch(std::ifstream::failure &err) {
		
		printf("readDumpFile:FAIL: %s", err.what());
		statusOk = false;
	}
	
	return statusOk;
}
