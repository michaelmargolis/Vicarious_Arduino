// Wave sketch (example 1)
#include <Vicarious.h>
#include <Servo.h>

const dataId_t HASTINGS = 1; // the id for the desired data stream
const int interval     = 100; // milliseconds between requests
const int SERVO_PIN    = 5;   // Pin connected to servo

Vicarious wave;  //create an instance of a vicarious data stream
Servo  myServo;   // create a servo instance

void setup()
{
  Serial.begin(9600); //debug messages are sent over USB
  wave.begin(HASTINGS);
  Serial.print("connecting ..");
  while ( wave.status() != DATA_AVAILABLE) {
    // here until a connection is valid and data is available
    Serial.print(".");
    delay(100);
  }
  wave.mapData(0, 255, 0, 180); // map all data to servo range
  myServo.attach(SERVO_PIN);
  Serial.println(" ready");
}

void loop()
{
  if (wave.status() == DATA_AVAILABLE) {
    int angle = wave.read();
    myServo.write(angle);
    delay(interval);
  }
}


