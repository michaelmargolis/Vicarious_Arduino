//  Wave collector and consumer sketch
// this version sends wave data as collector id 1, sequential values as id 2
// wave data received as vicarious id 10
// echo values received on id 20

#include <SoftwareSerial.h>
#include <Vicarious.h>


const int waveCollectorInterval = 100; // milliseconds between wave collector updates
const int hearbeatInterval      = 100;

VicariousCollector waveSensor;  //wave height sensor instance
VicariousCollector hearbeat;    // for testing

const int waveRxPin = 8;   // rx software serial
const int waveTxPin = -1;  // tx not used
SoftwareSerial softSerial(waveRxPin, waveTxPin); // RX, TX

VicariousConsumer waveConsumer; //create wave instance
const int WAVE_ID = 10;
const int consumerInterval      = 100; // milliseconds between consumer requests

void setup()
{
  Serial.begin(9600); //debug messages are sent over USB
  //while (!Serial);
  //delay(3000); // for debug only (time to for Serial to start up)

  softSerial.begin(9600);
  vicarious.begin();     // start the API

  Serial.print(PSTR("connecting .."));
  while ( vicarious.isConnected() == false) {
    // here until a connection is valid and data is available
    Serial.print(".");
    delay(100);
  }
  // start collectors
  waveSensor.begin(PSTR("Wave"));    // start the provider instance
  waveSensor.setInterval(waveCollectorInterval); // set time between data updates

  hearbeat.begin(PSTR("Sequence"));
  hearbeat.setInterval(hearbeatInterval);

  // start consumers
  if (! waveConsumer.begin(WAVE_ID)) {
    vicarious.pstrPrint(PSTR("begin failed for wave consumer\n"));
    delay(2000);
  }
  else {
    if ( !waveConsumer.smoothData(1)) { // samples so smooth
      vicarious.pstrPrint(PSTR("smoothing failed for wave\n"));
    }
    if ( !waveConsumer.mapData(0, 255, 0, 255)) {
      vicarious.pstrPrint(PSTR("mapData failed for wave\n"));
    }
    waveConsumer.setInterval(consumerInterval);   
  }
   waveConsumer.setInterval(consumerInterval);   
   vicarious.pstrPrint(PSTR("running\n"));
   delay(1000);
}

unsigned long collectorSequence = 1;
int rawWaveReading; // raw wave reading stored here
int rssi;  // signal strength reading stored here (but not used in this version)

void loop()
{
  // service the collector
  waveSensorRefresh();

  if (waveSensor.isSendTime() ) {
    int reading = getWaveReading();
    waveSensor.beginMsg();
    waveSensor.link.print(reading);
    waveSensor.endMsg();
  }
  if (hearbeat.isSendTime() ) {
    hearbeat.beginMsg();
    hearbeat.link.print(collectorSequence++);
    hearbeat.endMsg();
  }
  if (waveConsumer.isReadTime()) {
    int value = waveConsumer.read();
    Serial.print(millis());Serial.print(','); Serial.print(value); Serial.print(','); Serial.println(rawWaveReading);
  }
}

// call this function to refresh data from wave sensor
void waveSensorRefresh()
{
  static unsigned int unexpectedChars = 0;
  char c;
  static char buffer[13] = {0}; //rssi message requires null termination
  if (softSerial.available() >= 12)  {
    c = softSerial.read();
    if ( c == '-')
      return;
    if ( c == 'a') {
      for (byte i = 0; i < 11; i++) {
        buffer[i] = softSerial.read();
      }
      buffer[11] = '\0';
      //Serial.println(buffer);
      if ( strncmp(buffer, "--ANA", 5) == 0) {
        rawWaveReading = atoi(&buffer[5]);
        //Serial.print(millis()); Serial.print(": reading "); Serial.println(rawWaveReading);
      }
      else if ( strncmp(buffer, "MMRSSIM", 7) == 0) {
         rssi = atoi(&buffer[7]);
      }
      else {
        unexpectedChars++;
      }
    }
  }
  else {
    if ( c != -1  && c != 0) {
      unexpectedChars++;
      Serial.print("Got unexpected char: "); Serial.print((int)c);
      Serial.print(", count = "); Serial.println(unexpectedChars);
    }
  }
}

int getWaveReading()
{
  return rawWaveReading;  // convert to distance wave height here if needed
}

