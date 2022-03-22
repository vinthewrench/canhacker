//
//  CANbus.hpp
//  canrecord
//
//  Created by Vincent Moscaritolo on 3/18/22.
//

#pragma once

#if defined(__APPLE__)
// used for cross compile on osx
#include "can.h"

#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <thread>			//Needed for std::thread
#include <cstring>			//Needed for memset and string functions
#include <list>
#include <set>


using namespace std;
 


class CANbus {

public:
 
	CANbus();
	~CANbus();
	
	bool begin(const char *ifname, int *error = NULL);
	bool setFilter(canid_t can_id, canid_t can_mask, int *error = NULL);
	bool sendFrame(canid_t frameID,  uint8_t* data, size_t dataLen);
	void stop();
	
	virtual void rcvFrame(struct can_frame, long timeStamp ) = 0;

	bool isRunning() {return _running;};
	
protected:
	
	void run();

	bool 						_running;	 //Flag for starting and terminating the main loop
	std::thread 			_thread;		 //Internal thread, this is in order to start and stop the thread from
	
	
 	int						_fd;
	fd_set 					_read_fds;
	bool						_isSetup;

};
 
