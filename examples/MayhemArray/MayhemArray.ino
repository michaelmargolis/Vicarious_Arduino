// MayhemArray sketch
#include <Vicarious.h>


const int numberOfDataStreams = 14;

// the following array contains the data stream IDs to be used
const dataId_t dataStreamIds[numberOfDataStreams]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
byte data[numberOfDataStreams];                     //array to hold mayhem data

const int SMOOTHING    = 32; // number of samples to smooth
const int interval     = 100; // milliseconds between requests

Vicarious mayhem;  //create data stream instances

void setup()
{
  Serial.begin(57600); //debug messages are sent over USB
  if ( mayhem.begin(dataStreamIds, numberOfDataStreams)) {
    mayhem.smoothData(SMOOTHING);    
    Serial.print("connecting ..");
    while ( mayhem.status() != DATA_AVAILABLE) {
      // here until a connection is valid and data is available
      Serial.print(".");
      delay(100);
    }
    Serial.println(" ready");
  }
  else {
     Serial.println("Unable to connect!");
     while(true)
       ; // wait until arduino is restarted 
  }
}

void loop()
{
  if ( mayhem.read(data)) { // read data into the given data array
    for (int i = 0; i < numberOfDataStreams; i++ ) {
      setPixel(i, data[i]); // external function to set brightness of given LED
    }
    delay(interval);
  }
  else {
    // here if data could not be read
    Serial.println("unable to read data");
  }
}


// set brightness of the led at the given position to the given value
void setPixel(int position, int value)
{
  //to do
}

