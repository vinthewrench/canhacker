//
//  ScreenMgr.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//

#include "ScreenMgr.hpp"


ScreenMgr::ScreenMgr(){
	
}

ScreenMgr::~ScreenMgr(){
	
}


void ScreenMgr::clearScreen(){
	output("\x1b[H\x1b[2J");
}

void ScreenMgr::output(string str){
	printf("%s", str.c_str());
}

