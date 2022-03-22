//
//  CmdLineMgr.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//

#pragma once
#include <netinet/in.h>

#include <stdio.h>
#include <string>
#include <functional>
#include <vector>
#include <map>
 
#include "CommonDefs.hpp"
#include "CmdLineBuffer.hpp"
using namespace std;

class CmdLineMgr:  public CmdLineBufferManager {

public:
	CmdLineMgr();
	~CmdLineMgr();

	void clear();
	void start();
	void stop();
	bool isRunning() {return _isRunning;};
	
	void processChar(uint8_t c);

	void processChars(uint8_t* data, size_t len);


	// calls from CmdLineBufferManager
	virtual bool processCommandLine(std::string cmdLine, boolCallback_t cb);
	virtual stringvector matchesForCmd(const std::string cmd);
	virtual void helpForCommandLine(std::string cmdLine, boolCallback_t cb);
	virtual void quit() { stop(); };
		
	void sendReply(std::string reply);
	

	// used to verify
	void waitForChar( std::vector<char> chs,
						  std::function<void(bool didSucceed, char ch)> callback = NULL);

private:

	void registerBuiltInCommands();
	// built in commands
 	void doHelp(stringvector params);
	
 	CmdLineBuffer 		_cmdLineBuffer;
	bool 					_isRunning;
};
