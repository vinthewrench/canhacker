//
//  CanProtocol.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//

#pragma once

#include "CommonDefs.hpp"
#include <string>
typedef struct can_frame can_frame_t;

using namespace std;

class CanProtocol {
	
public:
 
	virtual string nameForFrame(can_frame_t frame) = 0;
	virtual string descriptionForFrame(can_frame_t frame) = 0;
  
private:
};

