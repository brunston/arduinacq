// this sketch is for tweaking soft i2c signals

// Uncomment next two lines to test fast software I2C
// #include <DigitalPin.h>
// #define USE_FAST_I2C_MASTER 1

#include <I2cMaster.h>

const uint8_t sdaPin = 8;
const uint8_t sclPin = 7;

#if USE_FAST_I2C_MASTER
FastI2cMaster<sdaPin, sclPin, false> i2c;
#else  // USE_FAST_I2C_MASTER
SoftI2cMaster i2c(sdaPin, sclPin);
#endif
// also test base class idea
I2cMasterBase *bus;

uint8_t mode;
//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);

  // convert softI2cMaster to TwoWireBase to test base class idea
  bus = &i2c;

  Serial.println("enter 0 for write, 1 for read, 2 for start/stop");
  while (!Serial.available());
  mode = Serial.read();
  if (mode == '0') {
    Serial.println("Write Mode");
  } else if (mode == '1') {
    Serial.println("Read Mode");
  } else if (mode == '2' ) {
    Serial.println("Start/Stop");
  } else {
    Serial.println("Bad Option");
    while(1);
  }
}
//------------------------------------------------------------------------------
void loop(void) {
  if (mode == '0') {
    bus->write(0X55);
  } else if (mode == '1') {
    bus->read(0);
  } else {
    bus->start(0XAA);
    bus->stop();
  }
  delay(1);
}