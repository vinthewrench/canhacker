//
//  main.cpp
//  canrecord
//
//  Created by Vincent Moscaritolo on 3/12/22.
//

 
#include "CmdLineMgr.hpp"
#include "CmdLineRegistry.hpp"
#include "CanBusMgr.hpp"
#include "FrameDB.hpp"
#include "GMLAN.hpp"
#include "Wranger2010.hpp"
#include "OBD2.hpp"

#include "FrameDumper.hpp"

#include <sstream>

#include <stdio.h>
#include <unistd.h>   // STDIN_FILENO, isatty(), ttyname()
#include <stdlib.h>   // exit()
#include <termios.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <execinfo.h>
#include <cassert>
#include <regex>

using namespace std;

void
handler()
{
	 void *trace_elems[20];
	 int trace_elem_count(backtrace( trace_elems, 20 ));
	 char **stack_syms(backtrace_symbols( trace_elems, trace_elem_count ));
	 for ( int i = 0 ; i < trace_elem_count ; ++i )
	 {
		  std::cout << stack_syms[i] << "\r\n";
	 }
	 free( stack_syms );

	 exit(1);
}


  
FrameDumper dumper;

static bool STOPCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	string errorStr;
	string portStr;

	string command = line[0];
	int  errnum = 0;

	if(line.size() > 1)
		portStr = line[1];

	CANBusMgr*	canBus = CANBusMgr::shared();
	
	dumper.stop();

	if(canBus->stop(portStr.c_str(),  &errnum)){
		mgr->sendReply( "OK");
		(cb)(true);
		return true;
	}
	else {
		errorStr = "Failed to stop to \x1B[36;1;4m"
				+ portStr + "\x1B[0m, Error: " + to_string(errnum)
				+ " " + string(strerror(errnum)) + " \r\n";
	}
	
	mgr->sendReply(errorStr);
	(cb)(false);
	return false;
};



static bool OBDCmdHandler( stringvector 		line,
								  CmdLineMgr* 		mgr,
								  boolCallback_t 	cb){
	string errorStr;
	string command = line[0];
	string cmdStr;
	string portStr;
	string canStr;
	CANBusMgr*	canBus = CANBusMgr::shared();

 	// my version only has OBD on can1
 
	portStr = "can1";
	canid_t can_id = 0x7DF;	// brodcast OBD

	
	if(line.size() > 1)
		cmdStr = line[1];

	if(line.size() > 2){
		canStr = line[2];
		
		if( ! regex_match(canStr, std::regex("^[A-Fa-f0-9]{3}"))
			||  !( std::sscanf(canStr.c_str(), "%x", &can_id) == 1)){
			errorStr =  "\x1B[36;1;4m"  + canStr + "\x1B[0m is not a valid a CAN ID. \r\n";
			goto done;
		}
	}

	std::transform(cmdStr.begin(), cmdStr.end(), cmdStr.begin(), ::tolower);
 
	if(cmdStr.empty() ) {
		errorStr =  "Command: \x1B[36;1;4m"  + command + "\x1B[0m expects command. \r\n";
	}
	else {
		int  errnum = 0;
		bool success = false;
		
		if(cmdStr == "vin"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x09, 0x02}, &errnum);
		}
		else if(cmdStr == "ecu"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x09, 0x0A}, &errnum);
		}
		else if(cmdStr == "pids"){
			
			for(uint8_t i = 0; i < 0xC0; i++){
				success = canBus->sendFrame(portStr, can_id, {0x02, 0x01, i, 0x00, 0x00, 0x00, 0x00, 0x00 }, &errnum);
				if(!success) break;
				usleep(10000);
			}
		}
		else if(cmdStr == "pid1"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, &errnum);
		}
		else if(cmdStr == "pid2"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x01, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00}, &errnum);
		}
		else if(cmdStr == "pid3"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00}, &errnum);
		}
		else if(cmdStr == "volts"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00}, &errnum);
		}
		else if(cmdStr == "fuel"){
			success = canBus->sendFrame(portStr, can_id, {0x02, 0x01, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00}, &errnum);
		}
		else if(cmdStr == "clearDTC"){
			success = canBus->sendFrame(portStr, can_id, {0x04}, &errnum);
		}
		else {
			errorStr =  "Command: \x1B[36;1;4m"  + cmdStr + "\x1B[0m is not valid \r\n";
			goto done;
		}
		
		if(success){
			mgr->sendReply( "OK");
			(cb)(true);
			
			return true;
		}
		
		errorStr = "Failed to write  to \x1B[36;1;4m" + portStr + "\x1B[0m, Error: " + string(strerror(errnum)) + " \r\n";
	}
	
done:
	mgr->sendReply(errorStr);
	(cb)(false);
	return false;
	
}

 
static bool MODECmdHandler( stringvector 		line,
								  CmdLineMgr* 		mgr,
								  boolCallback_t 	cb){
	string errorStr;
	string command = line[0];
	string modeStr;

	if(line.size() > 1)
		modeStr = line[1];

	if(modeStr.empty()) {
		errorStr =  "Command: \x1B[36;1;4m"  + command + "\x1B[0m expects a mode (FRAMES, VALUES, BOTH). \r\n";
	}
	else {
		FrameDumper::dump_mode_t mode = FrameDumper::NONE;
		
		if( modeStr =="both")
			mode = FrameDumper::BOTH;
		else if( modeStr =="values")
			mode = FrameDumper::VALUES;
		else if( modeStr =="frames")
			mode = FrameDumper::FRAMES;
	else
		errorStr =  "\x1B[36;1;4m"  + modeStr + "\x1B[0m is not a valid mode (FRAMES, VALUES, BOTH). \r\n";

		if(mode != FrameDumper::NONE){
			dumper.setDumpMode(mode);
			mgr->sendReply( "OK");
			(cb)(true);

			return true;
		}
	}
 
	mgr->sendReply(errorStr);
	(cb)(false);
	return false;

}

static bool SNIFFCmdHandler( stringvector line,
										CmdLineMgr* mgr,
										boolCallback_t	cb){
	string portStr;
	string errorStr;
	string command = line[0];
	int  errnum = 0;

	if(line.size() > 1)
		portStr = line[1];


	if(portStr.empty()) {
		errorStr =  "Command: \x1B[36;1;4m"  + command + "\x1B[0m expects a CAN interface. \r\n";
	}
	else {
		
		CANBusMgr*	canBus = CANBusMgr::shared();
	
	
		if(canBus->start(portStr.c_str(),  &errnum)){
			dumper.start(portStr);

			mgr->sendReply( "OK");
			(cb)(true);

			return true;
 		}
		else {
			errorStr = "Failed to open to \x1B[36;1;4m"
					+ portStr + "\x1B[0m, Error: " + string(strerror(errnum)) + " \r\n";
 		}
	}
	
	mgr->sendReply(errorStr);
	(cb)(false);
	return false;
}

static bool READCmdHandler( stringvector line,
										CmdLineMgr* mgr,
									boolCallback_t	cb){
	string fileName;
	string errorStr;
	string command = line[0];
	int  errnum = 0;
	
	CANBusMgr*	canBus = CANBusMgr::shared();
	
	if(line.size() > 1)
		fileName = line[1];
	
	
	if(fileName.empty()) {
		errorStr =  "Command: \x1B[36;1;4m"  + command + "\x1B[0m expects a filename.";
	}
	else {
		FrameDB::shared()->clearFrames();
		FrameDB::shared()->clearValues();
		
		(cb)(true);
		
		dumper.start();
		bool success =  canBus->readFramesFromFile(fileName, &errnum,
																 [=] () {
			// callback when done.
			dumper.stop();
			mgr->sendReply( "\r\n");
			(cb)(true);
			fflush(stdout);
			});
		
		if(success){
			return true;
		}
		else {
			errorStr = "Failed to read from \x1B[36;1;4m"
			+ fileName + "\x1B[0m, " + string(strerror(errnum)) + " \r\n";
			mgr->sendReply(errorStr);
			(cb)(false);
			
			
		}
	}
	return false;
}


void registerCommandsLineFunctions() {
	
	// register command line commands
	auto cmlR = CmdLineRegistry::shared();
	
	cmlR->registerCommand("mode" ,	MODECmdHandler);
	cmlR->registerCommand("sniff" ,	SNIFFCmdHandler);
	cmlR->registerCommand("stop" ,	STOPCmdHandler);
	cmlR->registerCommand("read" ,	READCmdHandler);
	cmlR->registerCommand("OBD" ,		OBDCmdHandler);

	cmlR->registerCommand("clear", [=] (stringvector line,
													 CmdLineMgr* mgr,
													 boolCallback_t cb ){
		printf("\x1b[0;0H\x1b[2J");
		(cb) (true);
		return false;
	});
	
 }
 

 

int main(int argc, const char * argv[]) {
	
	std::set_terminate( handler );

	fflush(stdout);
  printf("\x1b[0;0H\x1b[2J");
  printf("Can Hacker!\r\n");

	GMLAN gmlan;
	Wranger2010 jeep;
	OBD2	obdii;

	FrameDB*	frameDB = FrameDB::shared();
	CANBusMgr*	canBus = CANBusMgr::shared();
	
	CmdLineMgr  cmdLineMgr;
	registerCommandsLineFunctions();

	frameDB->registerProtocol("can1", &gmlan);
	frameDB->registerProtocol("can1", &obdii);
	
	frameDB->registerProtocol("can0", &jeep);
	frameDB->registerProtocol("can0", &obdii);

	canBus->registerHandler("can0");
	canBus->registerHandler("can1");


	try {
		struct termios tty_opts_backup, tty_opts_raw;
		if (!isatty(STDIN_FILENO)) {
			printf("Error: stdin is not a TTY\n");
			exit(1);
		}
		
		// Back up current TTY settings
		tcgetattr(STDIN_FILENO, &tty_opts_backup);
		
		// Change TTY settings to raw mode
		cfmakeraw(&tty_opts_raw);
 	//	tty_opts_raw.c_iflag |=  (IXON );
 //		tty_opts_raw.c_iflag &= ~(IGNBRK);
	

		tcsetattr(STDIN_FILENO, TCSANOW, &tty_opts_raw);
 
		//	struct winsize ws;
		//	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0  || ws.ws_col > 0) {
		//		printf("col : %d, rows: %hu \n\r",ws.ws_col,ws.ws_row);
		//		}
		
		cmdLineMgr.start([=](CmdLineMgr* cmd) {
			
			CANBusMgr*	canBus = CANBusMgr::shared();
			canBus->quitReading();
			
			dumper.stop();
	//		printf("Stopped...\r\n");
			cmd->reset();
		
		});
		
		while(cmdLineMgr.isRunning()){
			int c = getchar();
			cmdLineMgr.processChar(c);
		}
	 
		// Restore previous TTY settings
		tcsetattr(STDIN_FILENO, TCSANOW, &tty_opts_backup);
		dumper.stop();
	
	}
	catch ( const CanMgrException& e)  {
		printf("\tError %d %s\n\n", e.getErrorNumber(), e.what());
		return -1;
	}
	catch (std::invalid_argument& e)
	{
		printf("EXCEPTION: %s ",e.what() );
		return -1;
	}
	delete canBus;
	return EXIT_SUCCESS;
}
