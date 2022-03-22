//
//  ODBIIsocket.hpp
//  odbtool
//
//  Created by Vincent Moscaritolo on 3/19/22.
//

#ifndef ODBIIsocket_hpp
#define ODBIIsocket_hpp

#include "CANbus.hpp"
#include "OBDII.h"

using namespace std;



class ODBIIsocket  {
	
public:
	ODBIIsocket();
	~ODBIIsocket();
	
	bool begin(const char *ifname, canid_t tx_id, canid_t rx_id, int *error = NULL);
	void stop();
	
	OBDIIResponse performQuery(OBDIICommand *command);
	
	OBDIICommandSet getSupportedCommands();
	
private:

	int						_fd;
	bool						_isSetup;
	
};


#endif /* ODBIIsocket_hpp */
