//
//  FrameDB.cpp
//  canhacker
//
//  Created by Vincent Moscaritolo on 3/22/22.
//

#include "FrameDB.hpp"

FrameDB *FrameDB::sharedInstance = NULL;


FrameDB::FrameDB(){
	_lastEtag = 0;
	_interfaces.clear();
	_schema.clear();
	_values.clear();
}

FrameDB::~FrameDB(){
	
}
 

bool FrameDB::registerProtocol(string ifName, CanProtocol *protocol) {

	// create the interface if it doesnt already exist?
	if(_interfaces.count(ifName) == 0){
		interfaceInfo_t ifInfo;
		ifInfo.ifName = ifName;
		ifInfo.protocols.clear();
		ifInfo.frames.clear();
		_interfaces[ifName] = ifInfo;
	}

	// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto protoList = &m1->second.protocols;
	 
	// is it an already registered ?
	for(auto it = protoList->begin();  it != protoList->end(); ++it) {
		if(*it == protocol)
			return false;
	}
 
	protoList->push_back(protocol);
	protocol->registerSchema(this);
	
	return true;
}

void FrameDB::unRegisterProtocol(string ifName, CanProtocol *protocol){
	
	// erase all interfaces?
	if(_interfaces.count(ifName)){
		_interfaces.erase(ifName);
		_values.clear();
		return;
	}
	
	// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	if(m1 == _interfaces.end())
		return;
	
	auto protoList = &m1->second.protocols;
	// erase all protocols?
	if(!protocol){
		protoList->clear();
		return;
	}
	
	// erase it if registered  ?
	for(auto it = protoList->begin();  it != protoList->end(); ++it) {
		if(*it == protocol) {
			protoList->erase(it);
			return;
		}
	}
	
}


vector<CanProtocol*>	FrameDB::protocolsForInterface(string ifName){
	vector<CanProtocol*> protos;
	
	if(_interfaces.count(ifName)){
		auto m1 = _interfaces.find(ifName);
		if(m1 != _interfaces.end()){
			protos = m1->second.protocols;
		}
	}
	return protos;
}


// MARK: -  FRAMES
 

void FrameDB::clearFrames(string ifName){
	
	// close all?
	if(ifName.empty()){
		for (auto& [key, entry]  : _interfaces){
			entry.frames.clear();
		}
	}
	else for (auto& [key, entry]  : _interfaces){
		if (strcasecmp(key.c_str(), ifName.c_str()) == 0){
			entry.frames.clear();
			return;
		}
	}
	_lastEtag = 0;
}

 
void  FrameDB::saveFrame(string ifName, can_frame_t frame, long timeStamp){
	
	std::lock_guard<std::mutex> lock(_mutex);

	bitset<8> changed;
	bool isNew = false;
	long avgTime = 0;
	time_t now = time(NULL);
	
	canid_t can_id = frame.can_id & CAN_ERR_MASK;

	//Error check
	if(ifName.empty()) {
		throw CanMgrException("ifName is blank");
	}

	// create the interface if it doesnt already exist?
	if(_interfaces.count(ifName) == 0){
		interfaceInfo_t ifInfo;
		ifInfo.ifName = ifName;
		ifInfo.protocols.clear();
		ifInfo.frames.clear();
		_interfaces[ifName] = ifInfo;
	}
	
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;
	auto theProtocols = &m1->second.protocols;

	size_t count =  theFrames->count(can_id);
	if( count == 0){
		// create new frame entry
		frame_entry entry;
		entry.frame = frame;
//		entry.line = (uint) theFrames->size();
		entry.timeStamp = timeStamp;
		entry.avgTime = 0;
		entry.eTag = ++_lastEtag;
		entry.updateTime = now;
		entry.lastChange.reset();
		theFrames->insert( std::pair<canid_t,frame_entry>(can_id,entry));
		isNew = true;
//		line = entry.line;
	}
	else {
		// can ID is already there
		auto e = theFrames->find(can_id);
		auto oldFrame = &e->second.frame;
		
//		line = e->second.line;
		
		if(frame.can_dlc == oldFrame->can_dlc
			&& memcmp(frame.data, oldFrame->data, frame.can_dlc ) == 0){
			// frames are same - update timestamp and average
			long newAvg = ((timeStamp - e->second.timeStamp) + e->second.avgTime) / 2;
			e->second.timeStamp = timeStamp;
			e->second.avgTime = newAvg;
			avgTime =  newAvg;
		}
		else {
			// it changed
			//did the length change?
			if(frame.can_dlc != oldFrame->can_dlc) {
				for(int i = 0; i < frame.can_dlc; i++)
					changed.set(i);
			}
			else {
				for(int i = 0; i < frame.can_dlc; i++) {
					if(frame.data[i] != oldFrame->data[i])
						changed.set(i);
				}
			}
			
			// either way - update the frame
			// copy the frame
			memcpy( oldFrame,  &frame, sizeof(can_frame_t) );
			
			//- update timestamp and average
			long newAvg = ((timeStamp - e->second.timeStamp) + e->second.avgTime) / 2;
			e->second.timeStamp = timeStamp;
			e->second.avgTime = newAvg;
			e->second.eTag = ++_lastEtag;
			e->second.updateTime = now;
			e->second.lastChange = changed;
			avgTime =  newAvg;
		}
	}
	
	// tell the protocols something changed
 	if(isNew || (changed.count() > 0))
		for(auto proto : *theProtocols ){
			proto->processFrame(this, frame, now, _lastEtag );
		};
}


vector<canid_t> FrameDB::framesUpdateSinceEtag(string ifName, eTag_t eTag, eTag_t *eTagOut ){
	
	std::lock_guard<std::mutex> lock(_mutex);
	vector<canid_t> can_ids = {};

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
			throw CanMgrException("ifName name is not registered");
	}
	
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	
//	if(m1 == _interfaces.end()) {
//		throw CanMgrException("bug 1");
//
//	}
	auto theFrames = &m1->second.frames;
//	if(!theFrames) {
//		throw CanMgrException("bug 2");
//
//	}

	for (const auto& [canid, frame] : *theFrames) {
		if(frame.eTag <= eTag)
			can_ids.push_back(canid);
	}
	
	if(eTagOut)
		*eTagOut = _lastEtag;
	
	return can_ids;
}

vector<canid_t>  	FrameDB::framesOlderthan(string ifName, time_t time){
	vector<canid_t> can_ids = {};
	
	std::lock_guard<std::mutex> lock(_mutex);

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
		throw CanMgrException("ifName name is not registered");
	}
	
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;

	for (const auto& [canid, frame] : *theFrames) {
		if(frame.updateTime < time)
			can_ids.push_back(canid);
	}
	
	return can_ids;
}

bool FrameDB::frameWithCanID(string ifName, canid_t can_id, frame_entry *frameOut){
	
	frame_entry entry;
		
	std::lock_guard<std::mutex> lock(_mutex);

	// Does the interface exist?
	if(ifName.empty() || _interfaces.count(ifName) == 0) {
		throw CanMgrException("ifName name is not registered");
	}
		// get the map entry for that interface.
	auto m1 = _interfaces.find(ifName);
	auto theFrames = &m1->second.frames;

	if(theFrames->count(can_id) == 0 )
		return false;
	
	if(frameOut){
		auto e = theFrames->find(can_id);
		*frameOut =  e->second;
	}
 
	return true;
	
}


FrameDB::valueSchema_t FrameDB::schemaForKey(string_view key){
	valueSchema_t schema = {"", "", UNKNOWN};
 
	if(_schema.count(key)){
		schema =  _schema[key];
	}
	
	return schema;
}

void  FrameDB::clearValues(){
	_values.clear();
	_lastEtag = 0;
}

void FrameDB::addSchema(string_view key,  valueSchema_t schema){
	if( _schema.find(key) == _schema.end())
		_schema[key] = schema;
}
 

void FrameDB::updateValue(string_view key, string value, time_t when,  eTag_t eTag){
	
// 	cntr++;
	
	if(when == 0)
		when = time(NULL);

	_values[key] = {when, eTag, value};
}

vector<string_view> FrameDB::allValueKeys(){
	std::lock_guard<std::mutex> lock(_mutex);

	vector<string_view> keys;
	keys.clear();
	
	for (const auto& [key, value] : _values) {
			keys.push_back(key);
	}

	return keys;
}
  

vector<string_view> FrameDB::valuesUpdateSinceEtag(eTag_t eTag, eTag_t *newEtag){
	
	std::lock_guard<std::mutex> lock(_mutex);
	vector<string_view> keys = {};
	
	for (const auto& [key, value] : _values) {
		if(value.eTag <= eTag)
			keys.push_back(key);
	}

	return keys;
};

vector<string_view> FrameDB::valuesOlderthan(time_t time){
	
	std::lock_guard<std::mutex> lock(_mutex);
	vector<string_view> keys = {};
	
	for (const auto& [key, value] : _values) {
		if(value.lastUpdate < time)
			keys.push_back(key);
	}

	return keys;
};


bool FrameDB::valueWithKey(string_view key, string *valueOut){
	std::lock_guard<std::mutex> lock(_mutex);
	
	if(_values.count(key) == 0 )
		return false;

	if(valueOut){
		*valueOut = _values[key].value;
	}
 	
	return true;
};
