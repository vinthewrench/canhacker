//
//  ODBIIsocket.cpp
//  odbtool
//
//  Created by Vincent Moscaritolo on 3/19/22.
//

#include "ODBIIsocket.hpp"

#define MAX_ISOTP_PAYLOAD 4095

// Counts # bits set in the argument
// Code from Kernighan
static inline unsigned int _BitsSet(unsigned int word)
{
	unsigned int c; // c accumulates the total bits set in v
	for (c = 0; word; c++)
	{
	  word &= word - 1; // clear the least significant bit set
	}
	return c;
}

ODBIIsocket::ODBIIsocket(){
	_fd = -1;
	_isSetup = false;
}

ODBIIsocket::~ODBIIsocket(){
	
	stop();
	_fd = -1;
	_isSetup = false;

}

 

bool ODBIIsocket::begin(const char *ifname, canid_t tx_id, canid_t rx_id, int * errorOut){

	
	// https://github.com/hartkopp/can-isotp

	// create a isotp socket
		_fd = ::socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
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
		struct sockaddr_can addr;
		memset(&addr, 0, sizeof(addr));
		addr.can_family = AF_CAN;
		addr.can_ifindex = ifindex;
		addr.can_addr.tp.tx_id = tx_id;
		addr.can_addr.tp.rx_id = rx_id;
	 
		if (::bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			if(errorOut) *errorOut = errno;
			::close(_fd);
			return false;
		}
		
		_isSetup = true;
		return true;
};

void ODBIIsocket::stop(){
	if(_fd > -1){
		close(_fd);
	}
	_fd = -1;
	_isSetup = false;
}

 
OBDIIResponse ODBIIsocket::performQuery(OBDIICommand *command){
	
	OBDIIResponse response = { 0 };

	if(!_isSetup) {
		return response;
	}
	
	// Send the command
	int retval = (int) write(_fd, command->payload, sizeof(command->payload));
	if (retval < 0 || retval != sizeof(command->payload)) {
			return response;
	}

	// Set a one second timeout
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	fd_set readFDs;
	FD_ZERO(&readFDs);
	FD_SET(_fd, &readFDs);

	if (select(_fd + 1, &readFDs, NULL, NULL, &timeout) <= 0) {
		 // Either we timed out, or there was an error
		 return response;
	}

	int responseLength = command->expectedResponseLength == VARIABLE_RESPONSE_LENGTH ? MAX_ISOTP_PAYLOAD : command->expectedResponseLength;
	unsigned char responsePayload[responseLength];
	
	retval = (int) read(_fd, responsePayload, responseLength);

	
	if (retval < 0 || (command->expectedResponseLength != VARIABLE_RESPONSE_LENGTH && retval != command->expectedResponseLength)) {
		return response;
	}

	return OBDIIDecodeResponseForCommand(command, (unsigned char*) responsePayload, (int) retval);

};



 
OBDIICommandSet ODBIIsocket::getSupportedCommands(){
	OBDIICommandSet supportedCommands = { 0 };
	unsigned int numCommands;

	// Mode 1
	OBDIIResponse response = performQuery(OBDIICommands.mode1SupportedPIDs_1_to_20);

	supportedCommands._mode1SupportedPIDs._1_to_20 = response.bitfieldValue;

	// If PID 0x20 is supported, we can query the next set of PIDs
	if (!(response.bitfieldValue & 0x01)) {
		goto mode9;
	}

	response = performQuery(OBDIICommands.mode1SupportedPIDs_21_to_40);

	supportedCommands._mode1SupportedPIDs._21_to_40 = response.bitfieldValue;

	// If PID 0x40 is supported, we can query the next set of PIDs
	if (!(response.bitfieldValue & 0x01)) {
		goto mode9;
	}

	response = performQuery(OBDIICommands.mode1SupportedPIDs_41_to_60);

	// Mask out the rest of the PIDs, because they're not yet implemented
	response.bitfieldValue &= 0xFFFC0000;

	supportedCommands._mode1SupportedPIDs._41_to_60 = response.bitfieldValue;

	//// If PID 0x60 is supported, we can query the next set of commands
	//if (!(response.bitfieldValue & 0x01)) {
	//	goto mode9;
	//}

	//response = performQuery(OBDIICommands.mode1SupportedPIDs_61_to_80);

	//supportedCommands._mode1SupportedPIDs._61_to_80 = response.bitfieldValue;

mode9:
	// Mode 9
	response = performQuery(OBDIICommands.mode9SupportedPIDs);

	// Mask out the PIDs that are not yet implemented
	response.bitfieldValue &= 0xE0000000;

	supportedCommands._mode9SupportedPIDs = response.bitfieldValue;

exit:
	numCommands = _BitsSet(supportedCommands._mode1SupportedPIDs._1_to_20) + _BitsSet(supportedCommands._mode1SupportedPIDs._21_to_40) + _BitsSet(supportedCommands._mode1SupportedPIDs._41_to_60) + _BitsSet(supportedCommands._mode1SupportedPIDs._61_to_80) + _BitsSet(supportedCommands._mode9SupportedPIDs);

	numCommands += 2; // mode 1, pid 0 and mode 9, pid 0

	numCommands++; // mode 3

	OBDIICommand **commands = (OBDIICommand**) malloc(sizeof(OBDIICommand *) * numCommands);
	if (commands != NULL) {
		supportedCommands.commands = commands;
		supportedCommands.numCommands = numCommands;

		// Mode 1
		unsigned int pid;
		for (pid = 0; pid < sizeof(OBDIIMode1Commands) / sizeof(OBDIIMode1Commands[0]); ++pid) {
			OBDIICommand *command = &OBDIIMode1Commands[pid];
			if (OBDIICommandSetContainsCommand(&supportedCommands, command)) {
				*commands = command;
				++commands;
			}
		}

		// Mode 3
		*commands = OBDIICommands.DTCs;
		++commands;

		// Mode 9
		for (pid = 0; pid < sizeof(OBDIIMode9Commands) / sizeof(OBDIIMode9Commands[0]); ++pid) {
			OBDIICommand *command = &OBDIIMode9Commands[pid];
			if (OBDIICommandSetContainsCommand(&supportedCommands, command)) {
				*commands = command;
				++commands;
			}
		}
	}

	return supportedCommands;
}
