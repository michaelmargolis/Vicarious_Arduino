/*
 * Vicarious.cpp   
 * 
 * Copyright (C) 2014 Michael Margolis
 *
  * Updated Dec 2014 to add support for consumer int and float data types
 */


#include "Vicarious.h"

const long LINK_SPEED = 115200;
static HardwareSerial &link = LINK_STREAM;  // the serial stream talking to the gateway
  
#ifdef PRINTF_DEBUG
char _buf[64];
#endif

static void debugPrint(char* msg, long val)
{
   Serial.print(msg); Serial.println(val);
}

void dumpSerialBuffer()  // used for debug info only
{
    while(link.available() > 0 )
        Serial.write( link.read());
}

// status strings, Move this to program memory ?
char * vStatusStr[] = { "INVALID_DATASTREAM", "DATA_UNAVAILABLE", "DATA_AVAILABLE" };
 
Vicarious::Vicarious(){
  
}

boolean Vicarious::begin()
{
  return startLink();
  this->collectorCount = 0;
}

void Vicarious::setConsumerList(VicariousConsumer *list[])
{
  this->consumers = list;
}
 
void Vicarious::setProviderList(VicariousCollector *list[])
{
  this->providers = list;
}

boolean Vicarious::isConnected()
{
   return this->ready;
}
 
boolean Vicarious::startLink()
{
 if( !this->ready ) {
    link.begin(LINK_SPEED); 
    // Wait for U-boot to finish startup (this code from Bridge.cpp) todo!
    do {
      dropAll();
      delay(1000);
    } while (link.available() > 0); 
    this->ready = true;    
  }
  // todo add startup handshake to ensure link is valid??
  // perhaps validate the api and gateway versions are consistent
  return true; // todo return false if problem opening link!!!
}

void Vicarious::dropAll() {  
  while (link.available() > 0) {
    link.read();
  }
}

byte Vicarious::addCollector()
{
   this->collectorCount++;
   return this->collectorCount;
}

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

void Vicarious::pstrPrint(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) {
    Serial.write(c);  // todo
  }
}

Vicarious vicarious; // create an instance for the user

/**********************
   VicariousConsumer
**********************/
 
VicariousConsumer::VicariousConsumer() { }


boolean VicariousConsumer::begin(const consumerId_t id, vDataTypes type)
{

  this->dataId = id; 
  this->dataStreamCount = 1;
  sendBeginMessage(BEGIN_MSG_TAG, READ_MSG_TAG, this->dataId, type);
  return isReplySuccess();    
}

boolean VicariousConsumer::begin(const consumerId_t data[], const int count, vDataTypes type)
{
  this->dataStreamCount = count;
  boolean ret = false;
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(BEGIN_GROUP_MSG_TAG);  
  link.write(MSG_DELIM);
  link.write(READ_GROUP_MSG_TAG);
  link.write(MSG_DELIM);
  link.print(type);
  link.write(MSG_DELIM);
  link.print(count);
  for(byte i=0; i < count; i++) {
    link.write(MSG_DELIM);
    link.print(data[i]);
  } 
  link.print(MSG_TERMINATOR);
  
  consumerId_t id = getGroupId();
  if( id != 0) { 
     this->dataId = id;
     this->dataStreamCount = count;
     ret = true;
  }
  return ret;
}

consumerId_t VicariousConsumer::getId()
{
  return this->dataId;
}

byte VicariousConsumer::readByte()
{
  sendMessage(READ_MSG_TAG, dataId);
  long data; 
  if( getReplyValue(data) ) {   
     return (byte)data;
  } 
  debugPrint("error reading dataStream ", dataId);    
  return 0; // todo ??
}

int VicariousConsumer::readInt()
{
  sendMessage(READ_MSG_TAG, dataId);
  long data; 
  if( getReplyValue(data) ) {   
     return (int)data;
  } 
  debugPrint("error reading dataStream ", dataId);    
  return 0; // todo ??
}

long VicariousConsumer::readLong()
{
  sendMessage(READ_MSG_TAG, dataId);
  long data; 
  if( getReplyValue(data) ) {   
     return data;
  } 
  debugPrint("error reading dataStream ", dataId);    
  return 0; // todo ??
}

float VicariousConsumer::readFloat()
{
  sendMessage(READ_MSG_TAG, dataId);
  float data; 
  if( getReplyValue(data) ) {   
     return data;
  } 
  debugPrint("error reading dataStream ", dataId);    
  return 0.0; // todo ??
}

boolean VicariousConsumer::readBytes( byte data[])
{
  sendMessage(READ_GROUP_MSG_TAG, this->dataId, this->dataStreamCount); 
  if(  getReplyArray(data, this->dataStreamCount) ) {  
     return true;
  } 
  debugPrint("error reading chan ", this->dataId);        
  return false;
}

boolean VicariousConsumer::readInts( int data[])
{
  sendMessage(READ_GROUP_MSG_TAG, this->dataId, this->dataStreamCount); 
  if(  getReplyArray(data, this->dataStreamCount) ) {   
     return true;
  } 
  debugPrint("error reading chan ", this->dataId);        
  return false;
} 

boolean VicariousConsumer::readLongs( long data[])
{
  sendMessage(READ_GROUP_MSG_TAG, this->dataId, this->dataStreamCount); 
  if(  getReplyArray(data, this->dataStreamCount) ) {   
     return true;
  } 
  debugPrint("error reading chan ", this->dataId);        
  return false;
} 

boolean VicariousConsumer::readFloats( float data[])
{
  sendMessage(READ_GROUP_MSG_TAG, this->dataId, this->dataStreamCount); 
  if(  getReplyFloatArray(data, this->dataStreamCount) ) {   
     return true;
  } 
  debugPrint("error reading chan ", this->dataId);        
  return false;
}
 
vStatus_t VicariousConsumer::status()
{
  vStatus_t ret = INVALID_DATASTREAM;
  sendMessage(STATUS_MSG_TAG, this->dataId);
  if( isReplySuccess() ) {
     ret = DATA_AVAILABLE;
  } 
  else {
    debugPrint("error getting status for stream ", this->dataId);        
  }
  return ret; 
}

boolean VicariousConsumer::mapData(int fromLow, int fromHigh, int toLow, int toHigh )
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(MAP_MSG_TAG);
  link.write(MSG_DELIM);
  link.print(this->dataId);
  link.write(MSG_DELIM);
  link.print(fromLow);
  link.write(MSG_DELIM);
  link.print(fromHigh);
  link.write(MSG_DELIM);
  link.print(toLow);
  link.write(MSG_DELIM);
  link.print(toHigh);
  link.print(MSG_TERMINATOR);
  return isReplySuccess();
}

boolean VicariousConsumer::smoothData(int samples)
{
  sendMessage(SMOOTH_MSG_TAG,this->dataId, samples);
  return isReplySuccess();
}

void VicariousConsumer::sendMsgHeader(char header)
{
  vicarious.dropAll(); // clear out outstanding messages
  link.write(header);
}

void VicariousConsumer::sendMessage(const char tag, consumerId_t id)
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(tag);
  link.write(MSG_DELIM);
  link.print((int)id); // print the id as a number
  link.write(MSG_DELIM);
  link.print(MSG_TERMINATOR);
}

void VicariousConsumer::sendMessage(const char tag, consumerId_t id, long intVal)
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

void VicariousConsumer::sendBeginMessage(const char tag, const char tag2, consumerId_t id,  vDataTypes type)
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(tag);
  link.write(tag2);
  link.write(MSG_DELIM);
  link.print(id);
  link.write(MSG_DELIM);
  link.print(type); 
  link.write(MSG_DELIM);
  link.print(MSG_TERMINATOR);
}

#ifdef NOT_NEEDED
void VicariousConsumer::sendMessage(const char tag, consumerId_t id char *string)
{
  sendMsgHeader(REQUEST_MSG_HEADER);
  link.write(tag);
  link.write(MSG_DELIM);
  link.print(string); 
  link.print(MSG_TERMINATOR);
}
#endif

boolean VicariousConsumer::isMsgAvail()
{
  unsigned long startMillis = millis();
  while(link.available() < MIN_MSG_LEN) {
    //Serial.print(link.available());Serial.print(':');
    if( millis() - startMillis > READ_TIMEOUT) {
       debugPrint("msg timeout ", millis() - startMillis);
       dumpSerialBuffer();
       return false; // timeout waiting for message
    }
     // calls to serviceSensors(); could go here when needed 
  }
  return true; 
}

// returns true/false for messages that provide success/failure response codes
boolean VicariousConsumer::isReplySuccess()
{
  long result;
  if( getReplyValue(result) ) {     
     if( result == (int)REQUEST_SUCCESSFUL){
        return true;
     }
  }
  return false;    
}

// returns true with value set to the read value, else false with data = 0 
boolean VicariousConsumer::getReplyValue(long &value)
{
  char hdr,tag;  
  if( isMsgAvail() ) {
     if( (hdr=link.read()) == REPLY_MSG_HEADER) {     
        tag =link.read();
        //Serial.write((char)tag); Serial.print(",");
        //if( tag == READ_MSG_TAG) // ignore tag ?
        {                
          int id = link.parseInt();
          //Serial.print(id); Serial.print(",");
          long val = link.parseInt();
          //Serial.print(val); Serial.print(",");
          
          if(this->dataId == id) { // check if data for this instance
            value = val;    
            return true;
          }
          else{
            value = 0;
            debugPrint("getReplyValue ERR: on id ", dataId);   
         }    
       }
     }
  }
  return false;
}

// returns true with value set to the read value, else false with data = 0 
boolean VicariousConsumer::getReplyValue(float &value)
{
  char hdr,tag;
  //delay(20); // todo - change this to a while loop that exits when data is valid (or timeout)
  if( isMsgAvail() ) {
     if( (hdr=link.read()) == REPLY_MSG_HEADER) {     
        tag =link.read();
        //Serial.write((char)tag); Serial.print(",");
        //if( tag == READ_MSG_TAG) // ignore tag ?
        {                
          int id = link.parseInt();
          //Serial.print(id); Serial.print(",");
          float val = link.parseFloat();
          //Serial.print(val); Serial.print(",");
          
          if(this->dataId == id) { // check if data for this instance
            value = val;    
            return true;
          }
          else{
            value = 0;
            debugPrint("getReplyValue ERR: on id ", dataId);   
         }    
       }
     }
  }
  return false;
}

// returns the group id in response to a begin group msg (or 0 if valid reply not received) 
consumerId_t VicariousConsumer::getGroupId()
{
  char hdr,tag;
  consumerId_t groupId =0; 
  if( isMsgAvail() ) {
     if( (hdr=link.read()) == REPLY_MSG_HEADER) {     
        tag =link.read();
        if( tag == BEGIN_GROUP_MSG_TAG) 
        {                
          groupId = link.parseInt(); 
          //debugPrint("got group id:", groupId);          
       }
     }
  }
  return groupId;
}

// returns true with values array populated with read data, else false
// supports byte, int and long types
// note that values array will only be reliable when method returns true
template<typename TYPE>
boolean VicariousConsumer::getReplyArray(TYPE values[], byte count)
{
  char c; 
  if( isMsgAvail() ) {
     if( (c=link.read()) == REPLY_MSG_HEADER) {   
        if( (c=link.read()) == READ_GROUP_MSG_TAG) {                 
          int id = link.parseInt();           
          int msgCount = link.parseInt(); 
          if(this->dataId == id && msgCount == count) { // check if data for this instance           
            for(byte i=0; i < msgCount; i++) {              
              values[i] = (TYPE)link.parseInt();                   
            }
            return true;
          }
          else{ 
            debugPrint("getReplyValue ERR: on id ", dataId); 
            //printf("getReplyArray ERR: id=%d (%d) count=%d (%d)\n", this->dataId, id, count, msgCount);   
         }    
       }
     }
  }
  return false;
}

boolean VicariousConsumer::getReplyFloatArray(float values[], byte count)
{
  char c; 
  if( isMsgAvail() ) {
     if( (c=link.read()) == REPLY_MSG_HEADER) {   
        if( (c=link.read()) == READ_GROUP_MSG_TAG) {                 
          int id = link.parseInt();           
          int msgCount = link.parseInt(); 
          if(this->dataId == id && msgCount == count) { // check if data for this instance           
            for(byte i=0; i < msgCount; i++) {              
                 values[i] = link.parseFloat();                                  
            }
            return true;
          }
          else{ 
            debugPrint("getReplyValue ERR: on id ", dataId); 
            //printf("getReplyArray ERR: id=%d (%d) count=%d (%d)\n", this->dataId, id, count, msgCount);   
         }    
       }
     }
  }
  return false;
}

/*
boolean VicariousConsumer::getReplyArray(byte values[], byte count)
{
  char c; 
  if( isMsgAvail() ) {
     if( (c=link.read()) == REPLY_MSG_HEADER) {   
        if( (c=link.read()) == READ_GROUP_MSG_TAG) {                 
          int id = link.parseInt();           
          int msgCount = link.parseInt(); 
          if(this->dataId == id && msgCount == count) { // check if data for this instance           
            for(byte i=0; i < msgCount; i++) {
               values[i] = link.parseInt(); 
            }
            return true;
          }
          else{ 
            debugPrint("getReplyValue ERR: on id ", dataId); 
            //printf("getReplyArray ERR: id=%d (%d) count=%d (%d)\n", this->dataId, id, count, msgCount);   
         }    
       }
     }
  }
  return false;
}
*/

// returns true if it is time for a consumer to get data from gateway
boolean VicariousConsumer::isReadTime()
{
  unsigned long currentMillis = millis();
  if( this->readInterval > 0) {  // zero disables timed sending 
     if(currentMillis - this->previousReadTime >= this->readInterval) {           
        this->previousReadTime =  currentMillis; // reset the count
        return true;
     }
  }
  return false;  
}

void VicariousConsumer::setInterval(unsigned long dur)
{
  this->readInterval = dur;
  //debugPrint("consumer interval set to ", dur);  
}

/**********************
   VicariousCollector
**********************/
VicariousCollector::VicariousCollector() : link(LINK_STREAM){

}

boolean VicariousCollector::begin(PGM_P info)
{
  this->begin(info, 1);
}

boolean VicariousCollector::begin(PGM_P info, const byte count)
{
  this->info = info;
  this->count = count;
  this->id = vicarious.addCollector();
  Serial.print(info); Serial.print(" has id "); Serial.println(this->id);
 
  if(this->count > 1) {
    this->link.write(BEGIN_GROUP_MSG_TAG);
  }
  else {
    this->link.write(BEGIN_MSG_TAG);
  }
  this->link.write(WRITE_MSG_TAG);   
  link.write(MSG_DELIM);
  link.print(this->id);
  if(this->count > 1) {
    link.write(MSG_DELIM);
    link.print(this->count);
  }
  link.write(MSG_DELIM);
/* 
    this->dataId = id; 
  this->dataStreamCount = 1;
  sendMessage(BEGIN_MSG_TAG,'r', this->dataId, BYTE_TYPE);
  return isReplySuccess(); 
*/  
}

void VicariousCollector::beginMsg()
{
  this->link.write(REQUEST_MSG_HEADER);
  if(this->count <= 1)
    this->link.write(WRITE_MSG_TAG); 
  else  
    this->link.write(WRITE_GROUP_MSG_TAG);  
  link.write(MSG_DELIM);
  link.print(this->id);
  if(this->count > 1) {
    link.write(MSG_DELIM);
    link.print(this->count);
  }
  link.write(MSG_DELIM);
}

void VicariousCollector::endMsg()
{
  link.print(MSG_TERMINATOR);
}

void VicariousCollector::setInterval(long interval)
{
  this->sendInterval = interval;
  debugPrint("collector interval set to ", interval); 
}

// returns true if it is time for a collector to send data
boolean VicariousCollector::isSendTime()
{
  unsigned long currentMillis = millis();
  if( this->sendInterval > 0) {  // zero disables timed sending
     if(currentMillis - this->previousSendTime >= this->sendInterval) {           
        this->previousSendTime =  currentMillis; // reset the count
        return true;
     }
  }
  return false;  
}

collectorId_t VicariousCollector::getId()
{
    return this->id;
}
