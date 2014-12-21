// ConsumerProviderTest
// tests collection and consumption messages
#include <Vicarious.h>

#define CONSUMER_TEST

const int nbrConsumerChannels  = 1;
const int firstChannel         = 1; // the channel id of the first of sequential channels to use
const int consumerInterval     = 100; // milliseconds between consumer requests
VicariousConsumer consumer[nbrConsumerChannels];  //create channel instances
byte values[nbrConsumerChannels]; 


const int nbrCollectorChannels  = 1;
const int collectorInterval     = 10; // milliseconds between collector messages
VicariousCollector collector[nbrCollectorChannels];

const int SMOOTHING    = 1; // number of samples to smooth

// test stuff
unsigned long minRoundTripTime = 999999L; // set to min microseconds for read and reply
unsigned long maxRoundTripTime = 0;

unsigned long successCount = 0;
unsigned long errCount = 0;

void setup()
{
  DEBUG_STREAM.begin(115200); //debug messages 
  while(!Serial);
  delay(3000);  // wait until USB serial for debug has started
  vicarious.begin();     // start the API (link speed is set in library)
  DEBUG_STREAM.print("connecting ..");
  while ( vicarious.isConnected() == false) {
    // here until a connection is valid and data is available
    DEBUG_STREAM.print(".");
    delay(100);
  }

  while (true) // retry until all consumers and collecter messages succeed
  {
    boolean failed = false;
#ifdef CONSUMER_TEST    
    for (int i = 0; i < nbrConsumerChannels; i++ ) {
      int id = firstChannel + i;
      if (! consumer[i].begin(id)) {
        printf("begin failed for Id %d\n", id);
        failed = true;
      }
      else {
        if ( !consumer[i].smoothData(SMOOTHING)) {
          printf("smoothing failed for Id %d\n", id);
          failed = true;
        }
        if ( !consumer[i].mapData(0, 255, 0, 255)) {
          printf("mapData failed for Id %d\n", id);
          failed = true;
        }
        consumer[i].setInterval(consumerInterval);
      }
    }
#endif    
    for (int i = 0; i < nbrCollectorChannels; i++ ) {
      collector[i].begin(PSTR("Collector"));
      collector[i].setInterval(collectorInterval);
    }
    if (failed == false)
      break;
    delay(500);
  }
  vicarious.debug("All instances started ok");
}

// code to profile message times
static unsigned long startTime;
static unsigned long endTime;
static unsigned long activeElapsedMicros;
static unsigned long inactiveElapsedMicros;
static unsigned long activeElapsedMillis;
static unsigned long inactiveElapsedMillis;

void profile(boolean start)
{
   if(start) {
      startTime = micros();
      inactiveElapsedMicros = startTime - endTime;
      while(inactiveElapsedMicros > 1000) {
         inactiveElapsedMillis++;
         inactiveElapsedMicros -= 1000;
      }
   } 
   else {
      endTime = micros();
      activeElapsedMicros = endTime - startTime; 
      while(activeElapsedMicros > 1000) {
         activeElapsedMillis++;
         activeElapsedMicros -= 1000;
      }      
   }
}

void loop()
{
#ifdef CONSUMER_TEST    
  serviceConsumers();
#endif  
  serviceCollectors();
}

unsigned long consumerSequence = 1;
void serviceConsumers()
{  
  for (int i = 0; i < nbrConsumerChannels; i++ ) {
    if(consumer[i].isReadTime()) {
      profile(true);  // start to measure time to request and receive data
      values[i] = consumer[i].read();
      profile(false); // complete measurement 
     
      byte id = firstChannel + i;
      if (  values[i] != id) {
        errCount++;
        printf("data mismatch, got %d,  expected %d, errcount = %ld\n", values[i],id,errCount);
      }
      else {
        successCount++;
        if ( successCount % 100 == 0) {        
          long aveDur = activeElapsedMillis *1000/ successCount;
          //long percentActive = (activeElapsedMillis * 100) / inactiveElapsedMillis;
          sprintf( _buf, "After %ld replies, errs detected=%ld, ave msg dur=%ld\n", successCount, errCount, aveDur);
          vicarious.debug(_buf); 
        }
      }
    }    
  }
}

unsigned long collectorSequence = 1;

void serviceCollectors()
{  
  for (int i = 0; i < nbrCollectorChannels; i++ ) {
    if(collector[i].isSendTime()) {     
       collector[i].beginMsg();
       collector[i].link.print(collectorSequence++); 
       collector[i].endMsg();      
    }   
  }
}
