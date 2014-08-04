/*
 * Vicarious.h -  
 * 
 * Copyright (C) 2014 Michael Margolis
 */

#ifndef Vicarious_h
#define Vicarious_h

#include <Arduino.h>
#include <Stream.h>

#define PRINTF_DEBUG
#define VERBOSE_DEBUG(X) X  // uncomment x to enable verbose debug


/* Version numbers for the protocol.  
 * This number can be queried so that host software can test
 *  whether it will be compatible with the installed firmware. 
 */ 
const int VICARIOUS_MAJOR_VERSION  = 0; // for non-compatible changes
const int VICARIOUS_MINOR_VERSION  = 1; // for backwards compatibility
  
const char REQUEST_MSG_HEADER  = '>';  // precedes messages from API
const char REPLY_MSG_HEADER    = '<';  // precedes messages to API
const char ERROR_MSG_HEADER    = '~';  // error messages begin with this tag
const char DEBUG_MSG_HEADER    = '!';  // debug messages begin with this tag

// Command tags that follow message headers:
const char BEGIN_MSG_TAG       ='b';  // begin connection with the given data stream
const char BEGIN_GROUP_MSG_TAG ='B';  // begin connection with the list of data streams
const char STATUS_MSG_TAG      ='c';  // get connection status for the stream associated with the given tag
const char READ_MSG_TAG        ='r';  // read (get) data for the stream associated with the given tag
const char READ_GROUP_MSG_TAG  ='R';  // read (get) data for the stream associated with the given tag

const char SMOOTH_MSG_TAG     ='s';  // smooth data for the stream(s) associated with the given tag
const char MAP_MSG_TAG        ='m';  // map data for the stream(s) associated with the given tag

const long READ_TIMEOUT = 1000;  // max ms to wait for a response to a read request
const byte MIN_MSG_LEN = 5;      // valid request messages must be at least this many characters

const char MSG_DELIM      = '|';  // delimiter between message fields
const char MSG_TERMINATOR = '\n'; // 

typedef byte dataId_t;  // data stream identifier

// reply values from status request messages
enum vStatus_t {INVALID_DATASTREAM =-1, DATA_UNAVAILABLE, DATA_AVAILABLE};
// general purpose reply values
enum vReply_t {REQUEST_FAILED =-1, REQUEST_SUCCESSFUL = 1};
// enumeration of data types
enum vDataTypes { UNKNOWN_TYPE, BYTE_TYPE, INT16_TYPE, INT32_TYPE, FLOAT_TYPE, STRING_TYPE};




#ifdef PRINTF_DEBUG
extern char _buf[64];
#define printf(...)                         \
    Serial.write(DEBUG_MSG_HEADER);   \
    do {                            \
        sprintf(_buf, __VA_ARGS__); Serial.write(_buf); \
    } while (0) 
    
 #else
 #define printf(...) 
 #endif

class Vicarious 
{

public:
  Vicarious();    
  boolean begin(const dataId_t dataId);
  boolean begin(const dataId_t data[], const int count);
  byte read();
  boolean read( byte data[]);
  vStatus_t status();
  boolean mapData(int, int, int, int);
  boolean smoothData(int samples);
  void debug(const char *debugStr);

 private:
  dataId_t dataId;
  byte dataStreamCount;  // the number of streams connected to the dataId
  void sendMsgHeader(char header);
  void sendMessage(const char Tag, dataId_t id);
  void sendMessage(const char Tag,  dataId_t id, int intVal);
  //void sendMessage(const char tag, char *string); // not currently used
  void reportError(const char *errorStr);  // public??
  boolean getReplyResult();
  boolean getReplyValue(int &value);
  boolean getReplyArray(byte values[], byte count);
  boolean isMsgAvail();
  void startLink();
  void dropAll() ;
};

#endif