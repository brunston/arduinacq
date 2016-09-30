// Warning: set DS_RTC_USE_WIRE nonzero in SoftRTClib.h
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include <SoftRTClib.h>
#if !DS_RTC_USE_WIRE
#error set DS_RTC_USE_WIRE nonzero in SoftRTClib.h
#endif  // DS_RTC_USE_WIRE

RTC_DS1307 RTC;

void setup () {
  Serial.begin(9600);
  Wire.begin();
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