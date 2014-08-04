// Mayhem sketch
#include <Vicarious.h>

const int numberOfDataStreams = 14;
const int firstStream = 0; // the id of the first of sequential data stream to use

const int SMOOTHING    = 32; // number of samples to smooth
const int interval     = 100; // milliseconds between requests

Vicarious mayhem[numberOfDataStreams];  //create connection instances

void setup()
{
  Serial.begin(57600); //debug messages are sent over USB
  for (int i = 0; i < numberOfDataStreams; i++ ) {
    mayhem[i].begin(firstStream + i);
    mayhem[i].smoothData(SMOOTHING);
  }
  Serial.print("connecting ..");
  while ( mayhem[0].status() != DATA_AVAILABLE) {
    // here until a connection is valid and data is available for at least one channel
    Serial.print(".");
    delay(100);
  }
  Serial.println(" ready");
}

void loop()
{
  for (int i = 0; i < numberOfDataStreams; i++ ) {
    int val = mayhem[i].read();
    setPixel(i, val); // external function to set brightness of given LED
  }
  delay(interval);
}

// this function allows selection of ids
// when desired data streams are not sequentialy numbered
void beginSelectedStreams()
{
  // the following array contains the IDs to be used
  const int selection[] = { 1, 2, 5, 8, 9, 11, 12, 13, 15, 20, 21, 22, 23, 24};

  // a check to ensure the number of ids in the array is the same as the number of streams
  if (numberOfDataStreams  != sizeof(selection) / sizeof(int)) {
    Serial.print(F("wrong number of stream ids in selection array")) ;
  }
  else {
    for (int i = 0; i < numberOfDataStreams; i++ ) {
      mayhem[i].begin(selection[i]);
      mayhem[i].smoothData(SMOOTHING);
    }
  }
}

// set brightness of the led at the given position to the given value
void setPixel(int position, int value)
{
  //to do
}

