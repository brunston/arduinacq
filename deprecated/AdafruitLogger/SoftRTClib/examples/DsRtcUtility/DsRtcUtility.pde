// Utility sketch to set RTC time, date, and control registers.
//
// This sketch can be used with many Maxim I2C RTC chips.
// See your chip's data sheet.
//------------------------------------------------------------------------------
#include <avr/pgmspace.h>
#include <I2cMaster.h>
#include <SoftRTClib.h>

// use software I2C if USE_SOFT_I2C is nonzero
#define USE_SOFT_I2C 1

#if USE_SOFT_I2C
// use analog pins 4, 5 for software I2C
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Is Mega
const uint8_t RTC_SCL_PIN = 59;
const uint8_t RTC_SDA_PIN = 58;
#else
// Not Mega
const uint8_t RTC_SCL_PIN = 19;
const uint8_t RTC_SDA_PIN = 18;
#endif  // defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
SoftI2cMaster i2c(RTC_SDA_PIN, RTC_SCL_PIN);

#else  // USE_SOFT_I2C
// enable pull-ups on SDA/SCL
TwiMaster i2c(true);
#endif  // USE_SOFT_I2C

// constants for SQW test
uint8_t const SQW_DELAY = 10;
uint8_t const SQW_INTERRUPT = 0;
uint8_t const SQW_PIN = 2;
uint8_t const GPS_PIN = 3;
uint8_t const GPS_INTERRUPT = 1;
RTC_DS1307 rtc(&i2c);

uint32_t sqwCount;
int32_t gpsCount;
int32_t gpsSqwCount;

void gpsInterrupt();
bool hexReadEcho(uint8_t*);
void hexPrint(uint8_t);
void sqwInterrupt();
//------------------------------------------------------------------------------
#define printPSTR(s) print_P(PSTR(s))
//------------------------------------------------------------------------------
void print_P(PGM_P str) {
  uint8_t c;
  while ((c = pgm_read_byte(str++))) Serial.write(c);
}
//------------------------------------------------------------------------------
bool readUint8(uint8_t base, uint8_t *v) {
  uint16_t n = 0;
  while (!Serial.available()) {}
  while (Serial.available()) {
    uint8_t c = Serial.read();
    if ('0' <= c && c <= '9') {
      c -= '0';
    } else if ('a' <= c && c <= 'z') {
      c -= 'a' - 10;
    } else if ('A' <= c && c <= 'Z') {
      c -= 'A' - 10;
    } else {
      c = base;
    }
    n = base * n + c;
    if (c >= base || n >= 256) return false;
    delay(10);
  }
  *v = n;
  return true;
}
//------------------------------------------------------------------------------
bool decReadEcho(uint8_t min, uint8_t max, uint8_t* n) {
  uint8_t d;
  if (!readUint8(10, &d) || d < min || d > max) {
    printPSTR("Invalid\r\n");
    return false;
  }
  Serial.println(d, DEC);
  *n = d;
  return true;
}
//------------------------------------------------------------------------------
void displayTime(void) {
  DateTime dt;
  if (!rtc.getTime(&dt)) {
    printPSTR("DS1307 time error\r\n");
    return;
  }
  dt.printDdd(&Serial);
  printPSTR(", ");
  dt.printDateTime(&Serial);
  Serial.println();
}
//------------------------------------------------------------------------------
void helpDS1307Crtl() {
  printPSTR(
    "07h  DS1307 Control\r\n"
    "+---+---+---+----+---+---+---+---+\r\n"
    "|OUT| 0 | 0 |SQWE| 0 | 0 |RS1|RS2|\r\n"
    "| 0 | 0 | 0 |  0 | 0 | 0 | 1 | 1 |\r\n"
    "+---+---+---+----+---+---+---+---+\r\n"
    "OUT SQWE RS1 RS2  SQW Pin\r\n"
    " x    1   0   0   1 Hz\r\n"
    " x    1   0   1   4096 Hz\r\n"
    " x    1   1   0   8196 Hz\r\n"
    " x    1   1   1   32768 Hz\r\n"
    " 1    0   x   x   high\r\n"
    " 0    0   x   x   low\r\n");
}
//------------------------------------------------------------------------------
void helpDS3231Ctrl() {
  printPSTR(
    "0Eh  DS3231 Control\r\n"
    "+-----+-----+----+---+---+-----+----+----+\r\n"
    "|!EOSC|BBSQW|CONV|RS2|RS1|INTCN|A2IE|A1IE|\r\n"
    "|  0  |  0  |  0 | 1 | 1 |  1  |  0 |  0 |\r\n"
    "+-----+-----+----+---+---+-----+----+----+\r\n"
    "!EOSC - set to stop osc under battery\r\n"
    "BBSQW - set to enable SQW under battery\r\n"
    "CONV - set to start temp conversion\r\n"
    "RS2 RS1 SQW Pin\r\n"
    " 0   0     1 Hz\r\n"
    " 0   1  1024 Hz\r\n"
    " 1   0  4096 Hz\r\n"
    " 1   1  8196 Hz\r\n"
    "INTCN - clear to enable SQW, set for alarm\r\n"
    "A2IE - set to enable alarm 2 interrupt\r\n"
    "A1IE - set to enable alarm 1 interrupt\r\n");
}
//------------------------------------------------------------------------------
void helpDS3231CrtlStatus() {
  printPSTR(
    "0Fh  DS3231 Control/Status\r\n"
    "+---+---+---+---+-------+---+---+---+\r\n"
    "|OSF| 0 | 0 | 0 |EN32kHz|BSY|A2F|A1F|\r\n"
    "| 1 | 0 | 0 | 0 |   1   | x | x | x |\r\n"
    "+---+---+---+---+-------+---+---+---+\r\n"
    "OSF - osc was stopped flag\r\n"
    "EN32kHz - set to enable 32 kHz\r\n"
    "BSY - busy executing TCXO functions\r\n"
    "A2F - alarm 2 flag\r\n"
    "A1F - alarm 1 flag\r\n");
}
//------------------------------------------------------------------------------
void DS3231Temperature() {
  uint8_t r[2];
  const uint8_t DS3231_TEMP_ADD = 0X11;
  if (!rtc.read(DS3231_TEMP_ADD, r, 2)) {
    printPSTR("Read temp failed\r\n");
    return;
  }
  float T = ((r[0] << 8) | r[1]) / 256.0;
  Serial.print(T);
  printPSTR(" C\r\n");
}
//------------------------------------------------------------------------------
void dumpRegisters(void) {
  uint8_t a = 0;
  uint8_t h;
  do {
    if ((a % 8) == 0) {
      if (a) Serial.println();
      hexPrint(a);
      Serial.write(' ');
    }
    Serial.write(' ');
    if (!rtc.read(a, &h, 1)) break;
    hexPrint(h);
  } while(a++ != 0X3F);
  Serial.println();
}
//------------------------------------------------------------------------------
bool enableSQW() {
  uint8_t sqwEnable;
  printPSTR("Connect RTC signal to pin ");
  Serial.println(SQW_PIN, DEC);
  while (Serial.read() >= 0) {}
  printPSTR(
    "Type 1 to enable 32 kHz SQW on a DS1307\r\n"
    "Type 0 to skip SQW enable for chips like a DS3231\r\n"
    "Enter (0 or 1): ");
  if (!decReadEcho(0, 1, &sqwEnable)) return false;
  if (sqwEnable) {
    if (!rtc.setSQW(DS1307_SQW_32768_HZ)) {
      printPSTR("SQW enable failed\r\n");
      return false;
    }
  }
  return true;
}
//------------------------------------------------------------------------------
void gpsCal(void) {
  printPSTR("Connect GPS pps to pin ");
  Serial.println(GPS_PIN, DEC);
  if (!enableSQW()) return;
  printPSTR("\r\nType any character to stop\r\n\r\n");
  while (Serial.read() >= 0);
  pinMode(GPS_PIN, INPUT);
  pinMode(SQW_PIN, INPUT);
  // enable pullup
  digitalWrite(SQW_PIN, HIGH);
  digitalWrite(GPS_PIN, HIGH);
  // attach interrupts
  attachInterrupt(SQW_INTERRUPT, sqwInterrupt, FALLING);
  attachInterrupt(GPS_INTERRUPT, gpsInterrupt, FALLING);
  sqwCount = 0;
  delay(100);
  if (sqwCount == 0) {
    printPSTR("No RTC signal on pin ");
    Serial.println(SQW_PIN, DEC);
    return;
  }
  gpsCount = -1;
  while (Serial.available() == 0) {
    cli();
    int32_t s = gpsSqwCount;
    int32_t g = gpsCount;
    sei();
    if (g < 1) {
      printPSTR("Waiting for GPS pps on pin ");
      Serial.println(GPS_PIN, DEC);
      delay(1000);
    } else {
      float e = (float)(s - 32768 * g) / (32768 * g);
      Serial.print(g);
      if (e == 0.0) {
        printPSTR(" RTC == GPS \r\n");
      } else {
        if (e > 0) {
          printPSTR(" fast ");
        } else if (e < 0) {
          printPSTR(" slow ");
        }
        Serial.print(e * 24 * 3600, 3);
        printPSTR(" sec/day ");
        Serial.print(1000000 * e);
        printPSTR(" +-");
        Serial.print(0.02 + 2000000.0 / s);
        printPSTR(" ppm\r\n");
      }
      uint32_t m = millis();
      while (g == gpsCount) {
        if ((millis() -m) > 2000) gpsCount = -1;
      }
    }
  }
  detachInterrupt(GPS_INTERRUPT);
  detachInterrupt(SQW_INTERRUPT);
}
//------------------------------------------------------------------------------
void gpsInterrupt(void) {
  if (gpsCount < 0) {
    sqwCount = 0;
    gpsCount = 0;
    return;
  }
  gpsCount++;
  gpsSqwCount = sqwCount;
}
//------------------------------------------------------------------------------
void help() {
  printPSTR(
    "Options are:\r\n"
    "(0) Display date and time\r\n"
    "(1) Set date and time\r\n"
    "(2) Dump registers\r\n"
    "(3) Set register\r\n"
    "(4) DS3231 temperature\r\n"
    "(5) SQW/32kHz pin test\r\n"
    "(6) Calibrate with GPS pps\r\n"
    "(7) DS1307 control help\r\n"
    "(8) DS3231 control help\r\n"
    "(9) DS3231 control/status help\r\n");
}
//------------------------------------------------------------------------------
void hexPrint(uint8_t v) {
  Serial.print(v >> 4, HEX);
  Serial.print(v & 0XF, HEX);
}
//------------------------------------------------------------------------------
void hexPrintln(uint8_t v) {
  hexPrint(v);
  Serial.println();
}
//------------------------------------------------------------------------------
bool hexReadEcho(uint8_t* v) {
  uint8_t h;
  if (!readUint8(16, &h)) {
    printPSTR("Invalid\r\n");
    return false;
  }
  hexPrintln(h);
  *v = h;
  return true;
}
//------------------------------------------------------------------------------
void setDateTime(void) {
  DateTime dt;
  uint8_t Y, M, D, h, m, s;
  uint8_t v;
  printPSTR("Enter year [1,99]: ");
  if (!decReadEcho(0, 99, &Y)) return;
  printPSTR("Enter month [1,12]: ");
  if (!decReadEcho(1, 12, &M)) return;
  printPSTR("Enter day [1,31]: ");
  if (!decReadEcho(1, 31, &D)) return;
  printPSTR("Enter hour [0,23]: ");
  if (!decReadEcho(0, 23, &h)) return;
  printPSTR("Enter minute [0,59]: ");
  if (!decReadEcho(0, 59, &m)) return;
  printPSTR("Enter second [0,59]: ");
  if (!decReadEcho(0, 59, &s)) return;
  if (!dt.settime(2000 + Y, M, D, h, m, s)) {
    printPSTR("Invalid date/time\r\n");
    return;
  }
  if (!rtc.setTime(&dt)) {
    printPSTR("setTime failed\r\n");
    return;
  }
  displayTime();
}
//------------------------------------------------------------------------------
void setRegister(void) {
  uint8_t a;
  uint8_t r;
  printPSTR(
    "Address/Register\n\r"
    "7  DS1307 Control\n\r"
    "E  DS3231 Control\n\r"
    "F  DS3231 Control/Status\n\r"
    "10 DS3231 Aging Offset\n\r"
    "\n\r"
    "Enter address [0,FF]: ");
  if (!hexReadEcho(&a)) return;
  if (!rtc.read(a, &r, 1)) {
    printPSTR("read failed");
    return;
  }
  printPSTR("Current value: ");
  hexPrintln(r);
  printPSTR("q to quit or new value [0,FF]: ");
  while (!Serial.available());
  if (Serial.peek() == 'q') {
    Serial.println('q');
    return;
  }
  if (!hexReadEcho(&r)) return;
  if (!rtc.write(a, &r, 1)) {
    printPSTR("write failed");
  }
}
//------------------------------------------------------------------------------
void sqwInterrupt(void) {
  sqwCount++;
}
//------------------------------------------------------------------------------
void sqwTest(void) {
  if (!enableSQW()) return;
  // enable pullup
  digitalWrite(SQW_PIN, HIGH);
  printPSTR("Please wait ");
  Serial.print(SQW_DELAY, DEC);
  printPSTR(" seconds.\r\n");
  attachInterrupt(SQW_INTERRUPT, sqwInterrupt, FALLING);
  sqwCount = 0;
  delay(SQW_DELAY * 1000UL);
  detachInterrupt(SQW_INTERRUPT);
  Serial.print(sqwCount);
  printPSTR(" interrupts in ");
  Serial.print(SQW_DELAY, DEC);
  printPSTR(" seconds.\r\n");
  if (sqwCount == 0) {
    printPSTR("Is SQWE/EN32kHz set in the control register?\r\n");
    printPSTR("Is the RTC connected to pin ");
    Serial.print(SQW_PIN, DEC);
    Serial.println('?');
  }
}
//------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(9600);

  Serial.println();
  displayTime();
}
//------------------------------------------------------------------------------
void loop(void) {
  uint8_t n;

  while (Serial.read() >= 0);

  printPSTR("\r\nEnter option number or h for help: ");

  while (!Serial.available());
  n = Serial.read();
  Serial.write(n);
  Serial.println();
  Serial.println();

  switch (n) {
    case 'h':
    case 'H':
      help();
      break;

    case '0':
      displayTime();
      break;

    case '1':
      setDateTime();
      break;

    case '2':
      dumpRegisters();
      break;

    case '3':
      setRegister();
      break;

    case '4':
      DS3231Temperature();
      break;

    case '5':
      sqwTest();
      break;

    case '6':
      gpsCal();
      break;

    case '7':
     helpDS1307Crtl();
      break;

    case '8':
      helpDS3231Ctrl();
      break;

    case '9':
      helpDS3231CrtlStatus();
      break;

    default:
      printPSTR("Invalid option");
      break;
  }
}