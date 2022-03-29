//
//  main.cpp
//  canrecord
//
//  Created by Vincent Moscaritolo on 3/12/22.
//

 
#include "CmdLineMgr.hpp"
#include "CmdLineRegistry.hpp"
#include "ScreenMgr.hpp"

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


using namespace std;


//GMLAN	gmlan;
  
ScreenMgr screen;
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
		errorStr =  "Command: \x1B[36;1;4m"  + command + "\x1B[0m expects a CAN interface.";
	}
	else {
		
		CANBusMgr*	canBus = CANBusMgr::shared();
		
		if(canBus->start(portStr.c_str(),  &errnum)){
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
		string ifName = "";
		if( fileName.find("jeep") != string::npos)
			ifName = "can0";
		else if( fileName.find("chevy") != string::npos)
			ifName = "can1";
		else if( fileName.find("t0") != string::npos)
			ifName = "can1";
		else if( fileName.find("vin") != string::npos)
			ifName = "can1";

		FrameDB::shared()->clearFrames(ifName);
		FrameDB::shared()->clearValues();
	
		
	 	dumper.start(ifName);
		if( canBus->readFramesFromFile(fileName, &errnum)) {
			
 			dumper.stop();
			mgr->sendReply( "OK");
			(cb)(true);
			
	//		FrameDB::shared()->dumpValues();
			fflush(stdout);

			return true;
		}
		else {
			errorStr = "Failed to read from \x1B[36;1;4m"
			+ fileName + "\x1B[0m, " + string(strerror(errnum)) + " \r\n";
			
		}
	}
	
	mgr->sendReply(errorStr);
	(cb)(false);
	return false;
}

 


void registerCommandsLineFunctions() {
	
	// register command line commands
	auto cmlR = CmdLineRegistry::shared();
	
	cmlR->registerCommand("sniff" ,	SNIFFCmdHandler);
	cmlR->registerCommand("stop" ,	STOPCmdHandler);

	cmlR->registerCommand("read" ,	READCmdHandler);

	cmlR->registerCommand("clear", [=] (stringvector line,
													 CmdLineMgr* mgr,
													 boolCallback_t cb ){
		screen.clearScreen();
		(cb) (true);
		return false;
	});
	
 }
 

 

int main(int argc, const char * argv[]) {
	
	CANBusMgr*	canBus = CANBusMgr::shared();
	FrameDB*	frameDB = FrameDB::shared();

	CmdLineMgr  cmdLineMgr;
	registerCommandsLineFunctions();
	
	try {
		GMLAN gmlan;
 		Wranger2010 jeep;
		OBD2	obdii;
		
		frameDB->registerProtocol("can1", &gmlan);
		frameDB->registerProtocol("can1", &obdii);
		
		frameDB->registerProtocol("can0", &jeep);
		frameDB->registerProtocol("can0", &obdii);

		canBus->registerHandler("can0");
		canBus->registerHandler("can1");
		
		struct termios tty_opts_backup, tty_opts_raw;
		if (!isatty(STDIN_FILENO)) {
			printf("Error: stdin is not a TTY\n");
			exit(1);
		}
		
		// Back up current TTY settings
		tcgetattr(STDIN_FILENO, &tty_opts_backup);
		
		// Change TTY settings to raw mode
		cfmakeraw(&tty_opts_raw);
		tcsetattr(STDIN_FILENO, TCSANOW, &tty_opts_raw);
		
		
		//	struct winsize ws;
		//	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0  || ws.ws_col > 0) {
		//		printf("col : %d, rows: %hu \n\r",ws.ws_col,ws.ws_row);
		//		}
		
		screen.clearScreen();
		cmdLineMgr.start();
		
		while(cmdLineMgr.isRunning()){
			int c = getchar();
			cmdLineMgr.processChar(c);
		}
		
		
		// Restore previous TTY settings
		tcsetattr(STDIN_FILENO, TCSANOW, &tty_opts_backup);
		
	 
		
		
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
