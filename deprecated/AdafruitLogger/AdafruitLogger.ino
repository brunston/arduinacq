// A simple data logger for the Adafruit Data Logging shield on a mega
// You must edit SdFatConfig.h and set MEGA_SOFT_SPI nonzero
#include <SdFat.h>
#include <SdFatUtil.h>  // define FreeRam()
#include <I2cMaster.h>
#include <SoftRTClib.h>
#define CHIP_SELECT     10  // SD chip select pin
#define LOG_INTERVAL  1000  // mills between entries
#define SENSOR_COUNT     3  // number of analog pins to log
#define ECHO_TO_SERIAL   1  // echo data to serial port if nonzero
#define WAIT_TO_START    1  // Wait for serial input in setup()
#define ADC_DELAY       10  // ADC delay for high impedence sensors

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#if !MEGA_SOFT_SPI
#error set MEGA_SOFT_SPI nonzero in libraries/SdFat/SdFatConfig.h
#endif  // MEGA_SOFT_SPI
// Is a Mega use analog pins 4, 5 for software I2C
const uint8_t RTC_SCL_PIN = 59;
const uint8_t RTC_SDA_PIN = 58;
SoftI2cMaster i2c(RTC_SDA_PIN, RTC_SCL_PIN);

#elif defined(__AVR_ATmega32U4__)
#if !LEONARDO_SOFT_SPI
#error set LEONARDO_SOFT_SPI nonzero in libraries/SdFat/SdFatConfig.h
#endif  // LEONARDO_SOFT_SPI
// Is a Leonardo use analog pins 4, 5 for software I2C
const uint8_t RTC_SCL_PIN = 23;
const uint8_t RTC_SDA_PIN = 22;
SoftI2cMaster i2c(RTC_SDA_PIN, RTC_SCL_PIN);

#else  // defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Not Mega use hardware I2C
// enable pull-ups on SDA/SCL
TwiMaster i2c(true);
#endif  // defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

RTC_DS1307 RTC(&i2c); // define the Real Time Clock object
// file system object
SdFat sd;

// text file for logging
ofstream logfile;

// Serial print stream
ArduinoOutStream cout(Serial);

// buffer to format data - makes it eaiser to echo to Serial
char buf[80];
//------------------------------------------------------------------------------
#if SENSOR_COUNT > 6
#error SENSOR_COUNT too large
#endif  // SENSOR_COUNT
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
    DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
//------------------------------------------------------------------------------
// format date/time
ostream& operator << (ostream& os, DateTime& dt) {
  os << dt.year() << '/' << int(dt.month()) << '/' << int(dt.day()) << ',';
  os << int(dt.hour()) << ':' << setfill('0') << setw(2) << int(dt.minute());
  os << ':' << setw(2) << int(dt.second()) << setfill(' ');
  return os;
}
//------------------------------------------------------------------------------
void setup() {
  // For Leonardo
  while (!Serial) {}
  Serial.begin(9600);

  // pstr stores strings in flash to save RAM
  cout << endl << pstr("FreeRam: ") << FreeRam() << endl;

#if WAIT_TO_START
  cout << pstr("Type any character to start\n");
  while (Serial.read() < 0) {}
#endif  // WAIT_TO_START

  // connect to RTC
  if (!RTC.begin()) error("RTC failed");

  // set date time callback function
  SdFile::dateTimeCallback(dateTime);
  DateTime now = RTC.now();
  cout  << now << endl;

  // initialize the SD card
  if (!sd.begin(CHIP_SELECT)) sd.initErrorHalt();

  // create a new file in root, the current working directory
  char name[] = "LOGGER00.CSV";

  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    if (sd.exists(name)) continue;
    logfile.open(name);
    break;
  }
  if (!logfile.is_open()) error("file.open");

  cout << pstr("Logging to: ") << name << endl;

  // format header in buffer
  obufstream bout(buf, sizeof(buf));

  bout << pstr("millis");

  bout << pstr(",date,time");

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    bout << pstr(",sens") << int(i);
  }
  logfile << buf << endl;

#if ECHO_TO_SERIAL
  cout << buf << endl;
#endif  // ECHO_TO_SERIAL
}
//------------------------------------------------------------------------------
void loop() {
  uint32_t m;

  // wait for time to be a multiple of interval
  do {
    m = millis();
  } while (m % LOG_INTERVAL);

  // use buffer stream to format line
  obufstream bout(buf, sizeof(buf));

  // start with time in millis
  bout << m;

  DateTime now = RTC.now();
  bout << ',' << now;

  // read analog pins and format data
  for (uint8_t ia = 0; ia < SENSOR_COUNT; ia++) {
#if ADC_DELAY
    analogRead(ia);
    delay(ADC_DELAY);
#endif  // ADC_DELAY
    bout << ',' << analogRead(ia);
  }
  bout << endl;

  // log data and flush to SD
  logfile << buf << flush;

  // check for error
  if (!logfile) error("write data failed");

#if ECHO_TO_SERIAL
  cout << buf;
#endif  // ECHO_TO_SERIAL

  // don't log two points in the same millis
  if (m == millis()) delay(1);
}
