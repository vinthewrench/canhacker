//
//  CmdLineMgr.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//

#include "CmdLineMgr.hpp"
#include "CmdLineRegistry.hpp"
#include "Utils.hpp"


CmdLineMgr::CmdLineMgr(): _cmdLineBuffer(this) {
 	_cmdLineBuffer.clear();
 }

CmdLineMgr::~CmdLineMgr(){
	
}

void CmdLineMgr::start(){
	registerBuiltInCommands();
	_cmdLineBuffer.didConnect();
	_isRunning = true;
}

void CmdLineMgr::stop(){
	 sendReply("\r\n");
	_cmdLineBuffer.didDisconnect();
	_isRunning = false;
 }



void  CmdLineMgr::clear(){
	_cmdLineBuffer.clear();
}

void CmdLineMgr::processChar(uint8_t c){
	_cmdLineBuffer.processChars(&c, 1);
}


void CmdLineMgr::processChars(uint8_t* data, size_t len){
	_cmdLineBuffer.processChars(data, len);
}

 
bool CmdLineMgr::processCommandLine(std::string cmdLine, boolCallback_t completion){
	
	bool shouldRecordInHistory = false;

	CmdLineRegistry* reg = CmdLineRegistry::shared();
 
 
	bool hasColor =  true; //_server->_info.hasColor();
	
	vector<string> v = split<string>(cmdLine, " ");
	if(v.size() == 0)
		return(false);
		
	string cmd = v.at(0);
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
 
	auto cmCb = reg->handlerForCmd(cmd);
 
	if(cmCb) {
		sendReply("\r\n");
		shouldRecordInHistory = cmCb(v, this, completion);
	}
	else
	{
		string reply
			= string("\r\n  \x1B[2K")+ "command "
				+ ((hasColor)? "\x1B[36;1;4m" : "\"")
				+  cmd
				+ ((hasColor)? "\x1B[0m": "\"")
				+  " not found\r\n";
		
		sendReply(reply);
		clear();
		completion(true);
	}
	
	return shouldRecordInHistory;
}


void CmdLineMgr::helpForCommandLine(std::string cmdLine, boolCallback_t completion){
	
//	if(!_server->isConnected())
//		return;
//
	vector<string> v = split<string>(cmdLine, " ");

	doHelp(v);
	
	clear();
	completion(true);
 
}


void CmdLineMgr::registerBuiltInCommands(){
	
	CmdLineRegistry* reg = CmdLineRegistry::shared();
 
	// register any built in commands
	reg->registerCommand("history", [=] (stringvector line,
													 CmdLineMgr* mgr,
													 boolCallback_t completion ){
		mgr->_cmdLineBuffer.doCmdHistory(line);
		(completion) (true);
		return false;
	});
	

	reg->registerCommand("help", [=](stringvector line,
												CmdLineMgr* mgr,
												boolCallback_t cb){
		mgr->doHelp(line);
		(cb) (true);
		return false;
	});

	reg->registerCommand("bye", [=](stringvector line,
											  CmdLineMgr* mgr,
											  boolCallback_t cb){
		mgr->sendReply("  Well Bye...\r\n");
		(cb) (true);
		stop();
 		return false;	// dontt record in history

	});
}


stringvector CmdLineMgr::matchesForCmd( const std::string cmd){
	
	
	CmdLineRegistry* reg = CmdLineRegistry::shared();

	vector<string>  options = reg->matchesForCmd(cmd);

//	 	vector<string>  biglist =
//		{"false","file","findrule5.28","footprint","fsck_cs","functionnfc","filebyproc.d",
//			"finger","for","fsck_exfat","functions",
//			"fcgistarter","filecoordinationd","firmwarepasswd","foreach","fsck_hfs","funzip",
//			"fddist","fileproviderctl","fixproc","format-sql","fsck_msdos","fuser",
//			"fdesetup","filtercalltree","flex","format-sql5.18","fsck_udf","fuzzy_match",
//			"fdisk","find","flex++","format-sql5.28","fstyp","fwkdp",
//			"fg","find2perl","float","from","fstyp_hfs","fwkpfv",
//			"fgrep","find2perl5.18","fmt","fs_usage","fstyp_msdos",
//			"fi","findrule","fold","fsck","fstyp_ntfs",
//			"fibreconfig","findrule5.18","fontrestore","fsck_apfs","fstyp_udf"};
//
//		options.insert(end(options), begin(biglist), end(biglist));

	return options;
};



void CmdLineMgr::waitForChar( std::vector<char> chs,
									  std::function<void(bool didSucceed, char ch)> callback){

	_cmdLineBuffer.waitForChar(chs,callback);
}



// MARK: - built in commands
 

 
void CmdLineMgr::doHelp(stringvector params){
 
	CmdLineRegistry* reg = CmdLineRegistry::shared();

	auto helpStr = reg->helpForCmd(params);

	sendReply(helpStr);
}


 
void CmdLineMgr::sendReply(std::string str){
	printf("%s", str.c_str());
}
