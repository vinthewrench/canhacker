//
//  FrameDB.hpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//

#pragma once

#include "CommonDefs.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <mutex>
#include <bitset>
 
#include "CanProtocol.hpp"

using namespace std;

struct  frame_entry{
	can_frame_t 	frame;
//	uint				line;			// line number for this frame
	long				timeStamp;	// from canbus (tv.tv_sec - start_tv.tv_sec) * 100 + (tv.tv_usec / 10000);
	long				avgTime;		 // how often do we see these  ((now - lastTime) + avgTime) / 2
	eTag_t 			eTag;
	time_t			updateTime;
	bitset<8> 		lastChange;
};

class FrameDB {

	friend CanProtocol;
	
	public:
	
	static FrameDB *shared() {
		if(!sharedInstance){
			sharedInstance = new FrameDB;
		}
		return sharedInstance;
	}
 
	FrameDB();
	~FrameDB();
	
	bool registerProtocol(string ifName,  CanProtocol *protocol = NULL);
	void unRegisterProtocol(string ifName, CanProtocol *protocol);
	vector<CanProtocol*>	protocolsForInterface(string ifName);

	eTag_t lastEtag() { return  _lastEtag;};
	

// Frame database
	void saveFrame(string ifName, can_frame_t frame, long timeStamp);
	void clearFrames(string ifName);
	vector<canid_t>  	framesUpdateSinceEtag(string ifName, eTag_t eTag, eTag_t *newEtag);
	vector<canid_t>  	framesOlderthan(string ifName, time_t time);
	bool 					frameWithCanID(string ifName, canid_t can_id, frame_entry *frame);
	
// value Database
	
	typedef enum {
		INVALID = 0,
		BOOL,				// Bool ON/OFF
		INT,				// Int
		BINARY,			// Binary 8 bits 000001
		STRING,			// string
		
		PERCENT, 		// (per hundred) sign ‰
		MILLIVOLTS,		// mV
		MILLIAMPS,		// mA
		SECONDS,			// sec
		MINUTES,			// mins
		DEGREES_C,		// degC
		KPA,				// kilopascal
		PA,				// pascal

		DEGREES,			// Degrees (heading)

		VOLTS,			// V
		AMPS,				// A

		RPM,				// Rev per minute
		KPH,
		LPH,				// Liters / Hour
		GPS,				// grams per second
		KM,				// Kilometers
		RATIO,			//
		FUEL_TRIM,		// 
		SPECIAL,
		IGNORE,
		UNKNOWN,
	}valueSchemaUnits_t;

	typedef struct {
		string_view  		title;
		string_view  		description;
		valueSchemaUnits_t  	units;
	} valueSchema_t;


	void addSchema(string_view key,  valueSchema_t schema);
	valueSchema_t schemaForKey(string_view key);

	void updateValue(string_view key, string value, time_t when,  eTag_t eTag);
	void clearValues();
 
	vector<string_view> 		allValueKeys();
	vector<string_view>  	valuesUpdateSinceEtag(eTag_t eTag, eTag_t *newEtag);
	vector<string_view>  	valuesOlderthan(time_t time);
	bool 							valueWithKey(string_view key, string *value);
 
 protected:
 
private:
	
	static FrameDB *sharedInstance;
	
	mutable std::mutex _mutex;
	eTag_t _lastEtag;
	
	typedef struct {
		string							ifName;
		vector<CanProtocol*>   		protocols;
		map<canid_t,frame_entry> 	frames;
	} interfaceInfo_t;

// frames and interfaces
	map<string, interfaceInfo_t> _interfaces;
	
	// value database
	
	typedef struct {
		time_t			lastUpdate;
		eTag_t 			eTag;
		string			value;
		} value_t;

	map<string_view, valueSchema_t>			_schema;
	map<string_view, value_t> _values;
 
	
 };



