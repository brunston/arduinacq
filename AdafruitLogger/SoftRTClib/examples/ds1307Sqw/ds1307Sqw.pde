// demo of DS1307 SQW pin output
#include <I2cMaster.h>
#include <SoftRTClib.h>

// constants for SQW test
uint8_t const SQW_DELAY = 10;
uint8_t const SQW_INTERRUPT = 0;
uint8_t const SQW_PIN = 2;

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

RTC_DS1307 rtc(&i2c);

uint32_t sqwCount;

//------------------------------------------------------------------------------
void sqwInterrupt(void) {
  sqwCount++;
}
//------------------------------------------------------------------------------
void sqwTest(void) {
  pinMode(SQW_PIN, INPUT);
  // enable pullup
  digitalWrite(SQW_PIN, HIGH);
  Serial.print("Please wait ");
  Serial.print(SQW_DELAY, DEC);
  Serial.println(" seconds.");
  attachInterrupt(SQW_INTERRUPT, sqwInterrupt, FALLING);
  sqwCount = 0;
  delay(SQW_DELAY * 1000UL);
  detachInterrupt(SQW_INTERRUPT);
  Serial.print(sqwCount);
  Serial.print(" interrupts in ");
  Serial.print(SQW_DELAY, DEC);
  Serial.println(" seconds.");
  if (sqwCount == 0) {
    Serial.println("Is SQWE set in the control register?");
    Serial.print("Is SQW connected to pin ");
    Serial.print(SQW_PIN, DEC);
    Serial.println('?');
  }
}
//------------------------------------------------------------------------------
void setup () {
  Serial.begin(9600);
  Serial.print("Connect the DS1307 SQW pin to Arduino pin ");
  Serial.println(SQW_PIN, DEC);
  Serial.println("Type any character to count SQW interrupts");
  while (!Serial.available()) {}
  if (!rtc.setSQW(DS1307_SQW_32768_HZ)) {
    Serial.println("setSqw failed");
    return;
  }
  sqwTest();
  // disable SQW
  rtc.setSQW(DS1307_SQW_LOW);
}
//------------------------------------------------------------------------------
void loop() {}