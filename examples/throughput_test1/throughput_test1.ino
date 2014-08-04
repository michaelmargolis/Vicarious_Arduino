// throughput_test1
#include <Vicarious.h>

const int numberOfChannels = 14;
const int firstChannel = 1; // the channel id of the first of sequential channels to use

const int SMOOTHING    = 1; // number of samples to smooth
const int interval     = 50; // milliseconds between requests

Vicarious test[numberOfChannels];  //create channel instances
byte values[numberOfChannels];

// test stuff
unsigned long minRoundTripTime = 999999L; // set to min microseconds for read and reply
unsigned long maxRoundTripTime = 0;

unsigned long successCount = 0;
unsigned long errCount = 0;

void setup()
{
  Serial.begin(115200); //debug messages are sent over USB
  while (!Serial) ;
  waitKey();
  do {
    errCount = 0;
    for (int i = 0; i < numberOfChannels; i++ ) {
      int id = firstChannel + i;
      if (! test[i].begin(id)) {
        Serial.print("begin failed for Id "); Serial.println(id);
        errCount++;
      }
      else {
        if ( !test[i].smoothData(SMOOTHING)) {
          Serial.print("smoothing failed for Id "); Serial.println(id);
        }
        if ( !test[i].mapData(0, 255, 0, 255)) {
          Serial.print("mapData failed for Id "); Serial.println(id);
        }
      }
    }
  }
   while (errCount);
      if (errCount) {
        Serial.println("Stopping");
        while (1)
          ;
      }
    Serial.print("connecting ..");
    while ( test[0].status() != DATA_AVAILABLE) {
      // here until a connection is valid and data is available for at least one channel
      Serial.print(".");
      delay(100);
    }
    Serial.println(" ready");
    waitKey();
  }


  void loop()
  {
    unsigned long startTime = micros();
    for (int i = 0; i < numberOfChannels; i++ ) {
      //int id = (int)test[i].channelId;
      //printf("chan %d with ID %d status = %d", i, id, test[i].status());
      values[i] = test[i].read();
    }
    unsigned long dur = micros() - startTime;
    unsigned long durPerChan = dur / numberOfChannels;
    for (int i = 0; i < numberOfChannels; i++ ) {
      byte id = firstChannel + i;
      if (  values[i] != id) {
        errCount++;
        Serial.print("data mismatch, got "); Serial.print(values[i]); Serial.print(", expected "); Serial.print(id), Serial.print(", errcount= ");
        Serial.println(errCount);
      }
      else {
        successCount++;
        if ( successCount % 1000 == 0) {
          Serial.print("After "); Serial.print(successCount); Serial.print(" messages, errs detected= ");  Serial.println(errCount);
        }
      }
    }
    if (durPerChan < minRoundTripTime) {
      Serial.print("Round trip min time is");  Serial.print(durPerChan);  Serial.print(", Max is ");  Serial.println(maxRoundTripTime);
      minRoundTripTime = durPerChan;
    }
    if (durPerChan > maxRoundTripTime) {
      Serial.print("Round trip min time is");  Serial.print(durPerChan);  Serial.print(", Max is ");  Serial.println(maxRoundTripTime);
      maxRoundTripTime = durPerChan;
    }
    delay(interval);
  }

  char waitKey()
  {
    return 0;
    Serial.println("send any char to continue");
    while (Serial.available() < 1)
      ;
    return (char)Serial.read();
  }
