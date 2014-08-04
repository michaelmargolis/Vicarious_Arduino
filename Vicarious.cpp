/*
 * Vicarious.cpp   
 * 
 * Copyright (C) 2014 Michael Margolis
 */


#include "Vicarious.h"

const long LINK_SPEED = 115200;
static HardwareSerial & link = Serial1;  // the serial stream talking to the gateway
static boolean started;

#ifdef PRINTF_DEBUG
char _buf[64];
#endif

// status strings, Move this to program memory ?
char * vStatusStr[] = { "INVALID_DATASTREAM", "DATA_UNAVAILABLE", "DATA_AVAILABLE" };
 
Vicarious:: Vicarious(){
  
}
 
void Vicarious::startLink()
{
 if( !started ) {
    link.begin(LINK_SPEED);	
	// Wait for U-boot to finish startup (this code from Bridge.cpp) todo!
    do {
      dropAll();
      delay(1000);
    } while (link.available() > 0); 
	started = true;    
  }
}

boolean Vicarious::begin(const dataId_t id)
{
  startLink();
  dataId = id; 
  sendMessage(BEGIN_MSG_TAG, dataId, BYTE_TYPE);
  return getReplyResult();
}

boolean Vicarious::begin(const dataId_t data[], const int count)
{
  boolean ret = false;
  startLink();
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(BEGIN_GROUP_MSG_TAG);  
  link.write(MSG_DELIM);
  link.write(BYTE_TYPE);
  link.write(MSG_DELIM);
  link.print(count);
  for(byte i=0; i < count; i++) {
    link.write(MSG_DELIM);
    link.print(data[i]);
  } 
  link.print(MSG_TERMINATOR);
  
  int id;
  if( getReplyValue(id) ) {	
     dataId = (vStatus_t)id;
	 dataStreamCount = count;
	 ret = true;
  }
  return ret;
}

#ifdef NOT_NEEDED  
boolean Vicarious::begin(char *channelName)
{
  boolean ret = false;
  startLink();
  sendMessage(BEGIN_MSG_TAG,channelName); 
  int id;
  if( getReplyValue(id) ) {	
     dataId = (vStatus_t)id;
	 ret = true;
  }
  return ret;
}
#endif

byte Vicarious::read()
{
 // printf("\nREAD- ");
  sendMessage(READ_MSG_TAG, dataId);
  int data; 
  if( getReplyValue(data) ) {	
     return (byte)data;
  } 
  printf("error reading dataStream %d\n", dataId);	  
  return 0; // todo ??
}


boolean Vicarious::read( byte data[])
{

  sendMessage(READ_GROUP_MSG_TAG, dataId, dataStreamCount); 
  if( getReplyArray(data, dataStreamCount) ) {	
     return true;
  } 
  printf("error reading chan %d\n", dataId);	    
  // todo - zero the array before returning ???
  return false;
}
  
vStatus_t Vicarious::status()
{
  //printf("\nSTATUS- ");
  sendMessage(STATUS_MSG_TAG, dataId);
  int reply;
  if( getReplyValue(reply) ) {	
     return (vStatus_t)reply; 
  }
  printf("error getting status for  chan %d\n", dataId);	    
  return INVALID_DATASTREAM; 
}

boolean Vicarious::mapData(int fromLow, int fromHigh, int toLow, int toHigh )
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(MAP_MSG_TAG);
  link.write(MSG_DELIM);
  link.print(dataId);
  link.write(MSG_DELIM);
  link.print(fromLow);
  link.write(MSG_DELIM);
  link.print(fromHigh);
  link.write(MSG_DELIM);
  link.print(toLow);
  link.write(MSG_DELIM);
  link.print(toHigh);
  link.print(MSG_TERMINATOR);
  return getReplyResult();
}

boolean Vicarious::smoothData(int samples)
{
  sendMessage(SMOOTH_MSG_TAG,dataId, samples);
  return getReplyResult();
}

void Vicarious::sendMsgHeader(char header)
{
  dropAll(); // clear out outstanding messages
  link.write(header);
}

void Vicarious::sendMessage(const char tag, dataId_t id)
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(tag);
  link.write(MSG_DELIM);
  link.print((int)id); // print the id as a number
  link.write(MSG_DELIM);
  link.print(MSG_TERMINATOR);
  //printf("Sent msg for id %d (%d)\n", id, dataId);
}

void Vicarious::sendMessage(const char tag, dataId_t id, int intVal)
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(tag);
  link.write(MSG_DELIM);
  link.print(id);
  link.write(MSG_DELIM);
  link.print(intVal); 
  link.write(MSG_DELIM);
  link.print(MSG_TERMINATOR);
}

#ifdef NOT_NEEDED
void Vicarious::sendMessage(const char tag, dataId_t id char *string)
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(tag);
  link.write(MSG_DELIM);
  link.print(string); 
  link.print(MSG_TERMINATOR);
}
#endif

void Vicarious::debug(const char *debugStr)
{
  link.write(DEBUG_MSG_HEADER);
  link.print(debugStr); 
  link.print(MSG_TERMINATOR);
} 

void Vicarious::reportError(const char *errorStr)
{
  link.write(ERROR_MSG_HEADER);
  link.print(errorStr); 
  link.print(MSG_TERMINATOR);
}

boolean Vicarious::isMsgAvail()
{
  unsigned long startMillis = millis();
  while(link.available() < MIN_MSG_LEN) {
    if( millis() - startMillis > READ_TIMEOUT) {
	   printf("timeout %l\n", millis() - startMillis);
	   return false; // timeout waiting for message
	}
     // calls to serviceSensors(); could go here when needed 
  }
  return true; 
}

// returns true/false for messages that provide success/failure response codes
boolean Vicarious::getReplyResult()
{
  int result;
  if( getReplyValue(result) ) {     
     if( result == (int)REQUEST_SUCCESSFUL){
	    return true;
     }
  }
  return false;    
}
 
// returns true with value set to the read value, else false with data = 0 
boolean Vicarious::getReplyValue(int &value)
{
  char hdr,tag;
  if( isMsgAvail() ) {
     if( (hdr=link.read()) == REPLY_MSG_HEADER) {	  
        tag =link.read();
      //if( tag == READ_MSG_TAG) // ignore tag 
        {	    		 
	      int id = link.parseInt(); 		  
	      int val = link.parseInt();
		  if(dataId == id) { // check if data for this instance		   
		    //printf("getReplyValue OK: id=%d, val=%d\n", id, val);
		    value = val;	
		    return true;
		  }
		  else{
		    value = 0;
			printf("getReplyValue ERR: chan=%d Rx id=%d, val=%d\n", dataId, id, val);	
		 }	  
	   }
	 }
  }
  return false;
}

// returns true with values array populated with read data, else false
// note that values array will only be reliable when method returns true
boolean Vicarious::getReplyArray(byte values[], byte count)
{
  char c; 
  if( isMsgAvail() ) {
     if( (c=link.read()) == REPLY_MSG_HEADER) {	  
        if( (c=link.read()) == READ_GROUP_MSG_TAG) {	    		 
	      int id = link.parseInt(); 		  
		  int msgCount = link.parseInt(); 
		  if(dataId == id && msgCount == count) { // check if data for this instance		   
		    for(byte i=0; i < msgCount; i++) {
		       values[i] = link.parseInt();	
			}
		    return true;
		  }
		  else{	
			printf("getReplyArray ERR: id=%d (%d) count=%d (%d)\n", dataId, id, count, msgCount);	
		 }	  
	   }
	 }
  }
  return false;
}

void Vicarious::dropAll() {  
  while (link.available() > 0) {
    link.read();
  }
}