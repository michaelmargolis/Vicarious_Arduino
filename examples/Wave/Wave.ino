// Wave sketch (example 1)
#include <Vicarious.h>
#include <Servo.h>

const consumerId_t HASTINGS = 1;   // the id for the desired data stream
const int consumerInterval  = 100; // milliseconds between requests
const int smoothing         = 8;   // number of samples to smooth

VicariousConsumer wave;  //create an instance of a vicarious data stream

const int SERVO_PIN = 5; // Pin connected to servo
Servo  myServo;          // create a servo instance

void setup()
{
  Serial.begin(9600);  //debug messages are sent over USB
  vicarious.begin();   // start the API (link speed is set in library)
  Serial.print("Connecting to Vicarious server");  
  while ( vicarious.isConnected() == false) {
    // here until a connection is valid and data is available
    Serial.print(".");
    delay(100);
  }
  if(wave.begin(HASTINGS, BYTE_TYPE)) {   
    Serial.print("Connecting to Hastings wave consumer");
    while (wave.status() != DATA_AVAILABLE) {
      // here until a connection is valid and data is available
      Serial.print(".");
      delay(100);
    }
    Serial.println(" ok");
    wave.smoothData(smoothing);   
    wave.setInterval(consumerInterval);
    wave.mapData(0, 255, 0, 180); // map all data to servo range
    myServo.attach(SERVO_PIN);
    Serial.println(" ready");
  }
}

void loop()
{
  if( wave.isReadTime() ) {
    if (wave.status() == DATA_AVAILABLE) {
      int angle = wave.readByte();
      myServo.write(angle);     
    }
  }
}



