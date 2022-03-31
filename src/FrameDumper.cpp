//
//  FrameDumper.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/24/22.
//

#include "FrameDumper.hpp"
#include <unistd.h>
#include <sys/time.h>

#define CAN_OBD_MASK 0x00000700U /* standard frame format (SFF) */

std::string hexDumpFrame(can_frame_t frame, bool isOld, bitset<8> changed) {
	
	char      	 lineBuf[256];
	char       	 *p = lineBuf;

	canid_t can_id = frame.can_id & CAN_SFF_MASK;
	
	const char* resetColor = "\x1b[0m";	// reset color
	const char* frameIDColor = "\x1b[0m";	// reset color
	
	if((can_id & CAN_OBD_MASK) == 0x700) {
		if(can_id == 0x7DF)
			frameIDColor = "\x1b[97m"; 	// white
		else if(can_id >= 0x7e0 && can_id <= 0x7E7) // OBD Request
			frameIDColor =  "\x1b[96m"; 	// white
		else 	if(can_id >= 0x7e8 && can_id <= 0x7EF) 	// OBD reply
			frameIDColor = "\x1b[95m"; 	// magenta
	}
	
	p += sprintf(p, "%s%03X%s [%d] ", frameIDColor,  frame.can_id, resetColor, frame.can_dlc);
	
	for (int i = 0; i < frame.can_dlc; i++){
			const char* useColor = "";
		
			if(isOld) useColor = "\x1b[33m";
			else if (changed.test(i))  useColor = "\x1b[91m";
	
			p += sprintf(p,"%s%02X\x1b[0m ",  useColor, frame.data[i]);
	}
		 
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p,"   ");
	p += sprintf(p,"  ");
	
	if(isOld) p += sprintf(p,"\x1b[33m");
	
	for (int i = 0; i < frame.can_dlc; i++)  {
		
		uint8_t c = frame.data[i] & 0xFF;
		if (c > ' ' && c < '~')
			*p++ = c ;
		else {
			*p++ = '.';
		}
	}
	for (int i = 7; i >=  frame.can_dlc ; i--) p += sprintf(p," ");
	p += sprintf(p,"\x1b[0m");
	
	*p++ = 0;
	return string(lineBuf);
}



FrameDumper::FrameDumper(){
	_ifName = "";
	_running = false;
	_didStop = false;
	_lastValueLine = 0;
	_lastFrameLine = 0;
	_frameLineMap.clear();
	_valueLineMap.clear();
	_showFrames = false;
	_showValues	= false;
	_topOffset = 3;
	

}
FrameDumper::~FrameDumper(){
	stop();
	
	 
// if (_thread.joinable())
//		_thread.join();

}

void FrameDumper::start(string ifName){
	
	// stop if in progress/
	
	stop();
	
	FrameDB* frameDB = FrameDB::shared();
	frameDB->clearValues();
	frameDB->clearFrames();
 
	_lastValueLine = 0;
	_lastFrameLine = 0;
	_frameLineMap.clear();
	_valueLineMap.clear();
	_showFrames = true;
	_lastEtag = 0;
	_ifName = ifName;
		
	_running = true;
	_didStop = false;
	_thread = std::thread(&FrameDumper::run, this);
	_thread.detach();
}

void FrameDumper::stop(){
	
	if(_running) {
		_running = false;
	 
		// wait for it..
		while(!_didStop) usleep(100);
		
		_ifName = "";
 		printf("\x1b[?25h");
		printf("\x1b[%d;0H ", _lastValueLine + 3);
		fflush(stdout);
	}
}

static float to_farenheit( float in){
		return ((in * 1.8)  + 32.0);
}


static void printFrame(int line, bool isUpdated, bool isOld, string_view key){
	FrameDB* frameDB = FrameDB::shared();
	string value = "";
	
	char buffer[256];
	char *p = buffer;
	
 	if(frameDB->valueWithKey(key, &value)){
		
		auto schema = frameDB->schemaForKey(key);
		
		switch(schema.units){
			case	FrameDB::BOOL:
			{
				bool val =  stoi(value) != 0;
				p += sprintf(p, "%s", val ?"YES":"NO");
			}
			break;
			
			case	FrameDB::FUEL_TRIM:
			{
				float trim =   stof(value);
				p += sprintf(p, "%1.1f%%",  trim);
			}
			break;
	
		
			case	FrameDB::VOLTS:
			{
				float volts =   stof(value);
				p += sprintf(p, "%1.2fV",  volts);
			}
				break;
	
			case	FrameDB::DEGREES_C:
			{
				int temp =  (int)to_farenheit( stof(value) - 40);
				p += sprintf(p, "%dºF",  temp);
			}
				break;
				
			case FrameDB::KPA:
			{
				double pres = stof(value);
				pres = pres * 0.1450377377;
				p += sprintf(p, "%d psi", (int) pres);
			}
			break;
				
			case FrameDB::RPM:
			{
				uint rpm = stoi(value);
				rpm = rpm /4;
				p += sprintf(p, "%d RPM",  rpm);

			}
			break;
	 
			case	FrameDB::KM:
			{
				double dist =   stof(value) * 0.621371;
				p += sprintf(p, "%0.1f mi",  dist);
			}
				break;
	
			case	FrameDB::KPH:
			{
				double dist =   stof(value) * 0.621371;
				p += sprintf(p, "%0.1f mi/hr",  dist);
			}
				break;

			case	FrameDB::BINARY:
			{
				
				unsigned long val = stoul(value);
				if(val < 256){
					for(int i = 7 ; i >= 0 ; i--)
						p += sprintf(p, "%c ", ((val >> i) & 1) ?'1':'0');
				}
			}
				break;
			default:
				p += sprintf(p, "%s", value.c_str());
		}
		
		*p++ = 0;
		
		printf("\x1b[%d;0H\x1b[2K", line);// erase till end of line
		
		const char* useColor = "";
		if (isOld)
			useColor = "\x1b[33m";
		else
			if (isUpdated)
				useColor = "\x1b[91m";
		
		
		printf("%30s: ", string(key).c_str());
		printf("%s%s \x1b[0m", useColor, buffer);
		printf("\x1b[0K");
		printf("\r\n");
	}
}

void FrameDumper::printChangedValues(int lastLine, bool redraw){
 
	int offset = 2;
 
	FrameDB* frameDB = FrameDB::shared();
	time_t now = time(NULL);

	auto allKeys = frameDB->allValueKeys();
	auto old_keys = frameDB->valuesOlderthan(now - 5);
	auto updated_keys = frameDB->valuesUpdateSinceEtag(_lastEtag, NULL);
 
	if(redraw){
		printf("\x1b[%d;0H\x1b[0J", lastLine );// erase till end of line
		// redarw all values
		_valueLineMap.clear();
		_lastValueLine = lastLine + offset;
		
		for(auto key :allKeys){
			string value = "";
			_valueLineMap[key] = _lastValueLine;
	 
			bool isOld = std::find(old_keys.begin(), old_keys.end(), key) != old_keys.end();
			bool isUpdated = std::find(updated_keys.begin(), updated_keys.end(), key) != updated_keys.end();
		 
			printFrame(_lastValueLine, isUpdated, isOld, key);
			_lastValueLine++;
		}
	}
	else {
		
		for(auto key :updated_keys){
			int line = 0;
			auto it =  _valueLineMap.find(key);
			if(it != _valueLineMap.end()) {
				line = it->second;
			}else {
				_valueLineMap[key] = _lastValueLine;
				line = _lastValueLine;
				_lastValueLine++;
			}

			bool isOld = std::find(old_keys.begin(), old_keys.end(), key) != old_keys.end();
			bool isUpdated = std::find(updated_keys.begin(), updated_keys.end(), key) != updated_keys.end();
		 
			printFrame(line, isUpdated, isOld, key);
			}
	}
}

void FrameDumper::printHeaderLine(){
	
	FrameDB* frameDB = FrameDB::shared();
	auto allKeys = frameDB->allValueKeys();

	printf("\x1b[%d;0H\x1b[2K", 0);
	printf("\x1b[2K can-ids:%d  values:%d",
			 _lastFrameLine, (int) allKeys.size() );

}

void FrameDumper::printChangedFrames(string ifName, bool redraw){
	
 	eTag_t newEtag = 0;
	bool addedLines = false;

	FrameDB* frameDB = FrameDB::shared();
	time_t now = time(NULL);
	
	auto new_tags = frameDB->framesUpdateSinceEtag(ifName, _lastEtag, &newEtag);
	auto old_tags = frameDB->framesOlderthan(ifName, now - 5);
 
	if(redraw) {
		new_tags = frameDB->allFrames(ifName);
		addedLines = true;
		newEtag = _lastEtag;
	}

	for(auto tag : new_tags){
		frame_entry frame;
		bool isOldFrame = find(old_tags.begin(), old_tags.end(), tag) != old_tags.end();
		
		if( frameDB->frameWithTag(tag, &frame)){
			
			int line = 0;
			auto it =  _frameLineMap.find(tag);
			if(it != _frameLineMap.end()) {
				line = it->second;
			}else {
				_frameLineMap[tag] = _lastFrameLine++;
				line = _lastFrameLine;
				addedLines = true;
			}
	 		
			long avgTime = abs(frame.avgTime);
			if(avgTime > 99999) avgTime = 99999;
			
			printf("\x1b[%d;0H %6ld %s",
					 line + _topOffset,
					 avgTime,
					 hexDumpFrame(frame.frame, isOldFrame, frame.lastChange).c_str());
			
			auto protos = frameDB->protocolsForTag(tag);
			for( auto proto : protos){
				auto str = proto->descriptionForFrame(frame.frame);
				if(!str.empty()){
					printf("   %s", str.c_str());
					break;
				}
			}
			printf("\x1b[0K");
		}
	}
	
	//  erase the last line if we added one
	if(addedLines){
		printf("\x1b[%d;0H\x1b[0K", _lastFrameLine + _topOffset);
	}
		
	printChangedValues(_lastFrameLine + _topOffset,  redraw || addedLines );
	printHeaderLine();
	
	_lastEtag = newEtag;
	
}

void FrameDumper::run() {
	
	// clear screen
	printf("\x1b[0;0H\x1b[J\x1b[?25l");

	while(_running){
		printChangedFrames(_ifName);
		usleep(500);
		}
	
	printChangedFrames(_ifName, true);
	
	_didStop = true;
}
