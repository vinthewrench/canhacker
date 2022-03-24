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

typedef uint64_t eTag_t;
#define MAX_ETAG UINT64_MAX

typedef std::function<void(bool didSucceed)> boolCallback_t;
typedef std::function<void()> voidCallback_t;
typedef std::vector<std::string> stringvector;


#include <stdexcept>

class CanMgrException: virtual public std::runtime_error {
	
protected:
	
	int error_number;               ///< Error Number
	unsigned line;						// line number
	const char* function	; 			//function name
public:
	
	/** Constructor (C++ STL string, int, int).
	 *  @param msg The error message
	 *  @param err_num Error number
	 */
	explicit CanMgrException(const std::string& msg, int err_num = 0):
	std::runtime_error(msg)
	{
		line = __LINE__;
		function = __FUNCTION__;
		error_number = err_num;
	}
	
	
	/** Destructor.
	 *  Virtual to allow for subclassing.
	 */
	virtual ~CanMgrException() throw () {}
	
	/** Returns error number.
	 *  @return #error_number
	 */
	virtual int getErrorNumber() const throw() {
		return error_number;
	}
};
 


#endif /* CommonDefs_h */
