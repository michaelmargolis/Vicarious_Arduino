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
  DEBUG_STREAM.begin(115200); //debug messages (link speed is set in library)
  while (!DEBUG_STREAM) ; // for leonardo
  //waitKey();
  do {
    errCount = 0;
    for (int i = 0; i < numberOfChannels; i++ ) {
      int id = firstChannel + i;
      if (! test[i].begin(id)) {
        printf("begin failed for Id %d\n", id);
        errCount++;
      }
      else {
        if ( !test[i].smoothData(SMOOTHING)) {
          printf("smoothing failed for Id %d\n", id);
        }
        if ( !test[i].mapData(0, 255, 0, 255)) {
          printf("mapData failed for Id %d\n", id);
        }
      }
    }
  }
   while (errCount);
      if (errCount) {
        printf("Stopping \n");
        while (1)
          ;
      }
    printf("connecting ..");
    while ( test[0].status() != DATA_AVAILABLE) {
      // here until a connection is valid and data is available for at least one channel
      Serial.print(".");
      delay(100);
    }
    printf(" ready\n");
    //waitKey();
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
        printf("data mismatch, got %d,  expected %d, errcount = %ld\n",values[i],errCount);
      }
      else {
        successCount++;
        if ( successCount % 1000 == 0) {
          printf("After %ld messages, errs detected= %ld\n", successCount, errCount);
        }
      }
    }
    if (durPerChan < minRoundTripTime) {
      minRoundTripTime = durPerChan;
      printf("Round trip min time is %ld us, Max is %ld\n", durPerChan, maxRoundTripTime);
   
    }
    if (durPerChan > maxRoundTripTime) {
      maxRoundTripTime = durPerChan;
     printf("Round trip min time is %ld us, Max is %ld\n", durPerChan, maxRoundTripTime);

    }
    delay(interval);
  }

  // this function requires the debug port to be different from the linkport
  char waitKey()
  {
    if( DEBUG_STREAM == LINK_STREAM)
       return 0; // only wait for a char on the debug port if not the same as link    
    Serial.println("send any char to continue");
    while (Serial.available() < 1)
      ;
    return (char)Serial.read();
  }
