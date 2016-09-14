// Warning only use this for hardware debug!
// See which addresses respond to a start condition.

#include <I2cMaster.h>

// select software or hardware i2c
#define USE_SOFT_I2C 0

#if USE_SOFT_I2C
// use analog pins 4 and 5 for this example
// this allows a 328 shield to be used on the Mega
// edit next two line to change pins
const uint8_t SDA_PIN = A4;
const uint8_t SCL_PIN = A5;

// An instance of class for software master
SoftI2cMaster i2c(SDA_PIN, SCL_PIN);
#else // USE_SOFT_I2C
// hardware master with pullups enabled
TwiMaster i2c(true);
#endif  // USE_SOFT_I2C

//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);

  uint8_t add = 0;

  // try read
  do {
    if (i2c.start(add | I2C_READ)) {
      Serial.print("Add read: 0X");
      Serial.println(add, HEX);
      i2c.read(true);
    }
    i2c.stop();
    add += 2;
  } while (add);

  // try write
  add = 0;
  do {
    if (i2c.start(add | I2C_WRITE)) {
      Serial.print("Add write: 0X");
      Serial.println(add, HEX);
    }
    i2c.stop();
    add += 2;
  } while (add);

  Serial.println("Done");
}
void loop(void){}
