// Date and time functions using a DS1307 RTC connected via I2C and SoftI2cMaster

#include <I2cMaster.h>
#include <SoftRTClib.h>

#if defined(__AVR_ATmega1280__)\
|| defined(__AVR_ATmega2560__)
// Mega analog pins 4 and 5
// pins for DS1307 with software i2c on Mega
#define SDA_PIN 58
#define SCL_PIN 59

#elif defined(__AVR_ATmega168__)\
||defined(__AVR_ATmega168P__)\
||defined(__AVR_ATmega328P__)
// 168 and 328 Arduinos analog pin 4 and 5
#define SDA_PIN 18
#define SCL_PIN 19

#else  // CPU type
#error unknown CPU
#endif  // CPU type

// An instance of class for software master
SoftI2cMaster i2c(SDA_PIN, SCL_PIN);

RTC_DS1307 RTC(&i2c);

void setup () {
  Serial.begin(9600);
}

void loop () {
  DateTime now = RTC.now();

  now.printIsoDateTime(&Serial);
  Serial.println();

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future (now.unixtime() + 7 * 86400L + 30);

  Serial.print(" now + 7d + 30s: ");
  future.printIsoDateTime(&Serial);
  Serial.println();

  Serial.println();
  delay(3000);
}
