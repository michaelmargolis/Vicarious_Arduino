// ConsumerTypeTests
// tests supported consumer types
#include <Vicarious.h>

const int consumerInterval     = 25; // milliseconds between consumer requests

const consumerId_t byteConsumerId =  100;
const consumerId_t intConsumerId  =  101;
const consumerId_t longConsumerId =  102;
const consumerId_t floatConsumerId = 103; 

VicariousConsumer byteConsumer; 
VicariousConsumer intConsumer; 
VicariousConsumer longConsumer; 
VicariousConsumer floatConsumer; 

const int SMOOTHING            = 8; // number of samples to smooth

const int numberOfDataStreams  = 14;
const consumerId_t dataStreamIds[numberOfDataStreams] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

// group arrays to hold each supported type
byte byteArray[numberOfDataStreams]; 
int  intArray[numberOfDataStreams]; 
long longArray[numberOfDataStreams]; 
float floatArray[numberOfDataStreams]; 

VicariousConsumer byteGroupConsumer;
VicariousConsumer intGroupConsumer;
VicariousConsumer longGroupConsumer;
VicariousConsumer floatGroupConsumer;

// test stuff
unsigned long minRoundTripTime = 999999L; // set to min microseconds for read and reply
unsigned long maxRoundTripTime = 0;

unsigned long successCount = 0;
unsigned long errCount = 0;

void setup()
{
  DEBUG_STREAM.begin(57600); //debug messages 
  while(!Serial);
  delay(3000);  // wait until USB serial for debug has started
  boolean failed = false;

  vicarious.begin();     // start the API (link speed is set in library)
  DEBUG_STREAM.print(F("connecting .."));
  while ( vicarious.isConnected() == false) {
    // here until a connection is valid and data is available
    DEBUG_STREAM.print(".");
    delay(100);
  }
  DEBUG_STREAM.print(F(" Vicarious link connected\n"));

  // begin consumer groups:   
  beginConsumerGroup( byteGroupConsumer, dataStreamIds, numberOfDataStreams,
  "Byte test", BYTE_TYPE, SMOOTHING, consumerInterval);

  beginConsumerGroup( intGroupConsumer, dataStreamIds, numberOfDataStreams,
  "Int test", INT16_TYPE, SMOOTHING, consumerInterval);

  beginConsumerGroup( longGroupConsumer, dataStreamIds, numberOfDataStreams,
  ":Long test", INT32_TYPE, SMOOTHING, consumerInterval);                      

  beginConsumerGroup( floatGroupConsumer, dataStreamIds, numberOfDataStreams,
  "Float test", FLOAT_TYPE, SMOOTHING, consumerInterval);                           

  // begin individual consumers

  if (! byteConsumer.begin(byteConsumerId, BYTE_TYPE )) {
    printf("begin failed for byte Id %d\n", byteConsumerId);
    failed = true;
  }

  if (! intConsumer.begin(intConsumerId, INT16_TYPE )) {
    printf("begin failed for int Id %d\n", intConsumerId);
    failed = true;
  }

  if (! longConsumer.begin(longConsumerId, INT32_TYPE )) {
    printf("begin failed for long Id %d\n", longConsumerId);
    failed = true;
  }

  if (! floatConsumer.begin(floatConsumerId, FLOAT_TYPE )) {
    printf("begin failed for flaot Id %d\n", floatConsumerId);
    failed = true;
  }

  if (failed == false) {
    vicarious.debug("All single instances started ok");
    Serial.println(F(" ready (Send any key on Serial monitor to begin)"));
  }

  // wait for any key on the serial monitor
  while( Serial.read() < 0)
    ;
}

void loop()
{
  unsigned long startTime = millis();
  unsigned long startCount = successCount;
  unsigned int requests = 0;
  boolean err = false;
  
  byte bdata = byteConsumer.readByte();    
  if( isMismatch((bdata != byteConsumerId), byteConsumerId )) { 
    printf(", got %d,  expected %d, errcount = %ld\n", bdata ,byteConsumerId,errCount);
  }
  else
    requests++;

  int idata = intConsumer.readInt();    
  if( isMismatch((idata != 100* intConsumerId), intConsumerId )) { 
    printf(", got %d,  expected %d, errcount = %ld\n", idata ,intConsumerId,errCount);
  }
  else
    requests++;

  long ldata = longConsumer.readLong();    
  if( isMismatch((ldata != (1000L * longConsumerId)), longConsumerId )) { 
    printf(", got %ld,  expected %ld, errcount = %ld\n", ldata ,longConsumerId,errCount);
  }
  else
    requests++;
    
  float fdata = floatConsumer.readFloat();  
  //if( isMismatch( abs(data - (floatConsumerId * 10000L)) > .001), floatConsumerId )) {     
  if( isMismatch((fdata  !=  (10000L * floatConsumerId)), floatConsumerId )) { 
    printf(", got %d,  expected %d, errcount = %ld\n", fdata ,floatConsumerId,errCount);
  }
  else
    requests++;
    
  // byte group test
  if ( byteGroupConsumer.readBytes(byteArray)) { // read data into the given data array
    int id = byteGroupConsumer.getId();
    for (int i = 0; i < numberOfDataStreams; i++ ) {
      if(isMismatch((byteArray[i] != i),id)) { // see READ_GROUP_MSG_TAG code in ConsumerCollectorTest.py
        printf(", got %d,  expected %d, errcount = %ld\n",byteArray[i], id + i ,errCount);
        err = true;
        break;
      }
    }
    if( !err)
      requests++;
  }
  else { Serial.println(F("unable to read byte group data")); }


  // int group test
  err = false;
  if ( intGroupConsumer.readInts(intArray)) { // read data into the given data array
    int id = intGroupConsumer.getId();
    for (int i = 0; i < numberOfDataStreams; i++ ) {
      if(isMismatch((intArray[i] != id+i),id)) { // see READ_GROUP_MSG_TAG code in ConsumerCollectorTest.py
        printf(", got %d,  expected %d, errcount = %ld\n",byteArray[i], id + i ,errCount);
        err = true;
        break;
      }
    }
    if( !err)
      requests++;
  }
  else { Serial.println(F("unable to read byte group data")); }

  
  // long group test
  err = false;
  if ( longGroupConsumer.readLongs(longArray)) { // read data into the given data array
    int id = longGroupConsumer.getId();
    for (int i = 0; i < numberOfDataStreams; i++ ) {
      if(isMismatch((longArray[i] != id+i),id)) { // see READ_GROUP_MSG_TAG code in ConsumerCollectorTest.py
        printf(", got %d,  expected %d, errcount = %ld\n",byteArray[i], id + i ,errCount);
        err = true;
        break;
      }
    }
    if( !err)
      requests++;
  }
  else { Serial.println(F("unable to read byte group data")); }

  // float group test 
  err = false;  
  if ( floatGroupConsumer.readFloats(floatArray)) { // read data into the given data array
    int id = floatGroupConsumer.getId();
    float val = id;
    for (int i = 0; i < min(numberOfDataStreams,4); i++ ) { // don't check more than 4
      if(isMismatch(( abs(floatArray[i] - val) > .001 ), id )) {      
        Serial.print(", got ");                 Serial.print(floatArray[i]); 
        Serial.print(", expected ");            Serial.print(val); 
        printf(", errcount = %ld\n",errCount);  Serial.print(" diff = "); 
        Serial.println(abs(floatArray[i] - val),5);
        err = true;
        break;
      }
      val = val / 4.0;     
    }
   if( !err)
      requests++;
  }
  else { Serial.println(F("unable to read float group data")); }   
  
  unsigned long dur = millis() - startTime;
  printf("%ld successful fields in %d requests took %ld ms\n", successCount - startCount, requests, dur);
  //delay(consumerInterval);  // uncomment to slow down requests
}

// utility function for reporting success or failure for group begin calls
void beginConsumerGroup( VicariousConsumer &consumer, const consumerId_t *Ids, int nbrStreams,
char *label, vDataTypes type, int smoothing, int interval)
{
  if(consumer.begin(Ids, nbrStreams, type)) {   
    printf("Connecting %s consumer group ..", label);
    while (consumer.status() != DATA_AVAILABLE) {
      // here until a connection is valid and data is available
      Serial.print(".");
      delay(100);
    }
    Serial.println(" ok");
    consumer.smoothData(smoothing);   
    consumer.setInterval(interval);
  }
  else {
    Serial.println(F("Unable to connect consumer group!"));
    while(true)
      ; // wait until arduino is restarted 
  }
}

boolean isMismatch( bool misMatch, int id)
{
  bool result = true;  // returns true if mismatch
  if(!misMatch) {
    successCount++; 
    if ( successCount % 100 == 0) {        
      sprintf( _buf, "After %ld fields, errs detected=%ld\n", successCount, errCount);
      Serial.println(_buf);
      vicarious.debug(_buf);        
    }
    result = false;
  } 
  else { 
    errCount++;
    printf(" data mismatch for id %d", id);      
  }
  return result;
}




