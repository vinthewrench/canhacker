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

class CmdLineMgr;

typedef std::function<void(CmdLineMgr *, uint8_t ch)> cmdLineMgrCallback_t;

class CmdLineMgr:  public CmdLineBufferManager {

public:
	CmdLineMgr();
	~CmdLineMgr();

	void reset();
	
	void clear();
	void start(cmdLineMgrCallback_t interruptCallback = NULL);
	void stop();
	bool isRunning() {return _isRunning;};
	
	void processChar(uint8_t c);
	void processChars(uint8_t* data, size_t len);

	// calls from CmdLineBufferManager
	virtual bool processCommandLine(std::string cmdLine, boolCallback_t cb);
	virtual stringvector matchesForCmd(const std::string cmd);
	virtual void quit() { stop(); };
	
	virtual void deviceControl(uint8_t c);
 
	void sendReply(std::string reply);
	

	// used to verify
	void waitForChar( std::vector<char> chs,
						  std::function<void(bool didSucceed, char ch)> callback = NULL);

private:

	void registerBuiltInCommands();
	 
 	CmdLineBuffer 		_cmdLineBuffer;
	bool 					_isRunning;
	cmdLineMgrCallback_t 	_interruptCallback;
};
