//
//  ScreenMgr.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/21/22.
//
#pragma once

 
#include <stdio.h>
#include <unistd.h>   // STDIN_FILENO, isatty(), ttyname()
#include <stdlib.h>   // exit()
#include <termios.h>

#include <string>

using namespace std;
 
class ScreenMgr {
public:

	ScreenMgr();
	~ScreenMgr();
	
	void output(string str);

	void clearScreen();
	
	
	
};

