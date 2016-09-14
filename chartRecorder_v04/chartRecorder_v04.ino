/***************************************************************************************
 Chart Recorder with
  ER-TFTM070-5 (LCD) from EastRising (bought from buydisplay.com). Depends on RA8875 library from Adafruit.
  2 Thermocouples (Adafruit MAX31855)
  Adafruit Data Logger Shield
 
 
 RA8875 communication, connect:
 
   TFTM070(40 pin)   Arduino UNO pin    Description
    1,2                                 GND
    3,4                                 VCC
    5                10                 SPI SELECT
    6                ICSP_MISO          SDO / SPI_MISO
    7                ICSP_MOSI          SDI / SPI_MOSI
    8                ICSP_SCK           CLK                
   33                2                  CTP_INT touch data ready be read 
                                        from FT5x06 touch controller
   34                A4                 I2C for FT5x06 (preconfigured)
   35                A5                 I2C for FT5x06 (preconfigured)

  Thermocouple 0
  Thermocouple 1
  
****************************************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"
#include "FT5x06.h"
#include "RTClib.h"
#include "Adafruit_MAX31855.h"

// set up variables TFT utility library functions:
#define RA8875_CS         7   // RA8875 chip select for ISP communication
#define CTP_INT           2    // touch data ready for read from FT5x06 touch controller
#define RA8875_RESET      9    // Adafruit library puts a short low reset pulse at startup on this pin. 
                               // Not relevant for TFTM070 according to doc.
 
Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
FT5x06 cmt = FT5x06(CTP_INT);

// For the screen position
#define X_MAX 800
#define Y_MAX 480
#define X_LEFT 19
#define X_RIGHT 791
#define Y_TOP 19
#define Y_BOT 461

// set up variables using the RTC utility library functions:
RTC_DS1307 RTC;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 10;    

// Thermocouple1 digital IO pins.
#define thermo0DO   14
#define thermo0CS   15
#define thermo0CLK  16
#define thermo1DO   17
#define thermo1CS   18
#define thermo1CLK  19
Adafruit_MAX31855 thermocouple0(thermo0CLK, thermo0CS, thermo0DO);
Adafruit_MAX31855 thermocouple1(thermo1CLK, thermo1CS, thermo1DO);

// Our logging interval in milliseconds
#define LOG_INTERVAL 500
File dataFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  startRTC();
  //startSD();
  //startTFT();

  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    //while (1) ;
  }

  // basic readout test, just print the current temp
  Serial.print("Internal Temp 0 = ");
  Serial.println(thermocouple0.readInternal());
  Serial.print("Internal Temp 1 = ");
  Serial.println(thermocouple1.readInternal());

 
  Serial.println("Setup done.");


}

void loop(){
    DateTime now = RTC.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);

    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
}

void startRTC() {
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}
