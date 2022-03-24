//
//  GMLAN.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/23/22.
//


#include "CanProtocol.hpp"
#pragma once
 
class GMLAN: public CanProtocol {
	
public:
   
	virtual string nameForFrame(can_frame_t frame);
	virtual string descriptionForFrame(can_frame_t frame);
  
private:
	
};


