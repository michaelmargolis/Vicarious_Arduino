/*
 * Vicarious.h -  
 * 
 * Copyright (C) 2014 Michael Margolis
 * 
 * Updated Dec 2014 to add support for consumer int and float data types
 */

#ifndef Vicarious_h
#define Vicarious_h

#include <Arduino.h>
#include <Stream.h>

#define DEBUG_STREAM Serial

#ifdef __AVR_ATmega32U4__
#define LINK_STREAM Serial1 //HardwareSerial is Serial1 on Leonardo and Yun
#else
#define LINK_STREAM Serial  // HardwareSerial is Serial on Uno 
#endif

#define PRINTF_DEBUG
#define VERBOSE_DEBUG(X) X  // uncomment x to enable verbose debug



/* Version numbers for the protocol.  
 * This number can be queried so that host software can test
 *  whether it will be compatible with the installed firmware. 
 */ 
const int VICARIOUS_MAJOR_VERSION  = 1; // for non-compatible changes
const int VICARIOUS_MINOR_VERSION  = 0; // for backwards compatibility
  
const char REQUEST_MSG_HEADER  = '>';  // precedes messages from API
const char REPLY_MSG_HEADER    = '<';  // precedes messages to API
const char ERROR_MSG_HEADER    = '~';  // error messages begin with this tag
const char DEBUG_MSG_HEADER    = '!';  // debug messages begin with this tag

// Command tags that follow message headers:
const char BEGIN_MSG_TAG       ='b';  // begin connection with the given data stream
const char BEGIN_GROUP_MSG_TAG ='B';  // begin connection with the list of data streams

const char STATUS_MSG_TAG      ='c';  // get connection status for the stream associated with the given tag
const char READ_MSG_TAG        ='r';  // consumer reading (getting) data for the stream associated with the given tag
const char READ_GROUP_MSG_TAG  ='R';  // consumer reading (getting) data for the streams associated with the given tag

const char WRITE_MSG_TAG        = 'w'; // provider sending a single data value
const char WRITE_GROUP_MSG_TAG  = 'W'; // provider sending a group of data values

const char SMOOTH_MSG_TAG     ='s';  // smooth data for the stream(s) associated with the given tag
const char MAP_MSG_TAG        ='m';  // map data for the stream(s) associated with the given tag

const char INFO_TAG           = '?'; // when sent from Arduino, gateway replies with API version information
                                     // when received by Arduino, reply will be a list of providers

const long READ_TIMEOUT = 1000;  // max ms to wait for a response to a read request
const byte MIN_MSG_LEN = 5;      // valid request messages must be at least this many characters

const char MSG_DELIM      = '|';  // delimiter between message fields
const char MSG_TERMINATOR = '\n'; // 

typedef int consumerId_t;  // data stream identifier
typedef byte collectorId_t; // collector data identifier

// reply values from status request messages
enum vStatus_t {INVALID_DATASTREAM =-1, DATA_UNAVAILABLE, DATA_AVAILABLE};
// general purpose reply values
enum vReply_t {REQUEST_FAILED =-1, REQUEST_SUCCESSFUL = 1};
// enumeration of data types
enum vDataTypes { UNKNOWN_TYPE, BYTE_TYPE, INT16_TYPE, INT32_TYPE, FLOAT_TYPE, STRING_TYPE};




#ifdef PRINTF_DEBUG
extern char _buf[64];
#define printf(...)                         \
    DEBUG_STREAM.write(DEBUG_MSG_HEADER);   \
    do {                            \
        sprintf(_buf, __VA_ARGS__); DEBUG_STREAM.write(_buf); \
    } while (0) 
    
 #else
 #define printf(...) 
 #endif


class VicariousConsumer 
{

public:
  VicariousConsumer();    
  //boolean begin(const consumerId_t dataId);
  boolean begin(const consumerId_t dataId, vDataTypes type);
  
  //boolean begin(const consumerId_t data[], const int count);
  boolean begin(const consumerId_t data[], const int count, vDataTypes type);
  
  byte readByte();
  int readInt();
  long readLong();
  float readFloat(); 


  boolean readBytes( byte data[]);
  boolean readInts( int data[]);
  boolean readLongs( long data[]);
  boolean readFloats( float data[]);


  vStatus_t status();
  boolean mapData(int, int, int, int);
  boolean smoothData(int samples);
  boolean isReadTime();
  void setInterval(unsigned long dur);
  consumerId_t getId(); // only needed for testing

 private:
  consumerId_t dataId;
  byte dataStreamCount;  // the number of streams connected to the dataId
  void sendMsgHeader(char header);
  void sendMessage(const char Tag, consumerId_t id);
  void sendMessage(const char Tag,  consumerId_t id, long longVal);
  //void sendMessage(const char Tag,  consumerId_t id, float floatVal); 
  
  void sendBeginMessage(const char tag, const char tag2, consumerId_t id, vDataTypes type);

  
  //void sendMessage(const char tag, char *string); // not currently used
  boolean isReplySuccess();
  consumerId_t getGroupId();
  
  boolean getReplyValue(long &value);
  boolean getReplyValue(float &value); 

  template<typename TYPE>
  boolean getReplyArray(TYPE values[], byte count);
  boolean getReplyFloatArray(float values[], byte count);
 // boolean getReplyArray(int values[], byte count);
 // boolean getReplyArray(long values[], byte count);


  
  
  boolean isMsgAvail();
  unsigned long readInterval;
  unsigned long previousReadTime;
};

class VicariousCollector 
{
public:
VicariousCollector(); 
boolean begin(PGM_P info);
boolean begin(PGM_P info, const byte count);
void beginMsg();
void endMsg();
void setInterval(long interval);
boolean isSendTime();
HardwareSerial &link;
collectorId_t getId();

private:
collectorId_t id;
unsigned long sendInterval;
unsigned long previousSendTime;
byte count; // the number of fields
PGM_P info;
};

class Vicarious 
{

public:
  friend class VicariousConsumer;
  friend class VicariousProducer;
  Vicarious();    
  boolean begin();
  void setConsumerList(VicariousConsumer *list[]); 
  void setProviderList(VicariousCollector *list[]); 
  void debug(const char *debugStr);
  void reportError(const char *errorStr);  // public??
  boolean isConnected();
  void pstrPrint(PGM_P str);
  byte addCollector(); 

 private:
  byte collectorCount;   //the number of collectors  
  boolean ready;
  VicariousConsumer ** consumers;
  VicariousCollector ** providers;
  boolean startLink();
  void dropAll() ;
};

extern Vicarious vicarious; // one global instance created in cpp file

#endif