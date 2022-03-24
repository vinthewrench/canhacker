//
//  CANFrame.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//

#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <mutex>


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

using namespace std;


typedef struct can_frame  can_frame_t;


inline  std::string hexDumpFrame(can_frame_t frame) {
	
	char      	 lineBuf[80];
	char       	 *p = lineBuf;

	p += sprintf(p, "%03X [%d] ",  frame.can_id, frame.can_dlc);
	
	for (int i = 0; i < frame.can_dlc; i++) p += sprintf(p,"%02X ",frame.data[i]);
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p,"   ");
	p += sprintf(p,"  ");
	for (int i = 0; i < frame.can_dlc; i++)  {
		
		uint8_t c = frame.data[i] & 0xFF;
		if (c > ' ' && c < '~')
			*p++ = c ;
		else {
			*p++ = '.';
		}
	}
	*p++ = 0;
	return string(lineBuf);
}

struct  timed_can_frame{
	canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
	uint8_t len;
	uint8_t data[CAN_MAX_DLEN] __attribute__((aligned(8)));
	
	uint32_t		number;		// how many of these did we see
	
	long		timeStamp;
	long		avgTime;		 // how often do we see these  ((now - lastTime) + avgTime) / 2
};
 

//
//class CANFrame {
//public:
//
//	CANFrame();
//	~CANFrame();
//
//	void saveFrame(can_frame_t frame, long timeStamp);
//	void clearFrames();
//	
//
//private:
//	map<canid_t, timed_can_frame>  frames;
//};
//

