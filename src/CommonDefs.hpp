//
//  CommonDefs.h

//
//  Created by Vincent Moscaritolo on 3/19/21.
//

#ifndef CommonDefs_h
#define CommonDefs_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <functional>
#include <vector>
#include <string>
 
#if defined(__APPLE__)
// used for cross compile on osx
#include "can.h"
typedef struct can_frame can_frame_t;

#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif


typedef std::function<void(bool didSucceed)> boolCallback_t;
typedef std::function<void()> voidCallback_t;
typedef std::vector<std::string> stringvector;


#endif /* CommonDefs_h */
