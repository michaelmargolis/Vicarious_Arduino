// MayhemFloatExample

#include <Vicarious.h>

const int consumerInterval     = 25; // milliseconds between consumer requests

VicariousConsumer mayhemConsumer; 

const int SMOOTHING            = 8; // number of samples to smooth

const int numberOfDataStreams  = 14;
const consumerId_t dataStreamIds[numberOfDataStreams] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

float floatArray[numberOfDataStreams]; 

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

  if(mayhemConsumer.begin(dataStreamIds, numberOfDataStreams, FLOAT_TYPE)) {   
    Serial.print(F("Connecting mayhem consumer group .."));
    while (mayhemConsumer.status() != DATA_AVAILABLE) {
      // here until a connection is valid and data is available
      Serial.print(".");
      delay(100);
    }
    Serial.println(" ok");
    mayhemConsumer.smoothData(SMOOTHING);   
    mayhemConsumer.setInterval(consumerInterval);
  }
  else {
    Serial.println(F("Unable to connect consumer group!"));
    while(true)
      ; // wait until arduino is restarted 
  } 
}

void loop()
{
  unsigned long startTime = millis();
  
  if ( mayhemConsumer.readFloats(floatArray)) { // read data into the given data array
    Serial.print("Read: ");   
    for (int i = 0; i < numberOfDataStreams; i++ ) { 
        Serial.print(floatArray[i],7); // show seven decimal places
        if( i < numberOfDataStreams-1)
           Serial.print(",");   
        else   
           Serial.println();
   }   
  }
  else { Serial.println(F("unable to read float group data")); }   
  
  unsigned long dur = millis() - startTime;
  printf("read dur was %ld ms\n", dur);
  //delay(consumerInterval);  // uncomment to slow down requests
}




