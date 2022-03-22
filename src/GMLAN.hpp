//
//  GMLAN.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//


#include "CANbus.hpp"
#include "CANFrame.hpp"

#pragma once

using namespace std;


class GMLAN : public CANbus, CANFrame {
	void rcvFrame(can_frame_t frame, long timeStamp );
};

 
