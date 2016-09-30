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
  startSD();
  startTFT();

  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }

  // basic readout test, just print the current temp
  Serial.print("Internal Temp 0 = ");
  Serial.println(thermocouple0.readInternal());
  Serial.print("Internal Temp 1 = ");
  Serial.println(thermocouple1.readInternal());

 
  Serial.println("Setup done.");


}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";
  String sensorString = "";

  DateTime now = RTC.now();
  dataString += String(now.year());
  dataString += "/";
  dataString += String(now.month());
  dataString += "/";
  dataString += String(now.day());
  dataString += " ";
  dataString += String(now.hour());
  dataString += ":";
  dataString += String(now.minute());
  dataString += ":";
  dataString += String(now.second());
  dataString += ",";

  dataString += String(thermocouple0.readInternal());
  dataString += ",";

  dataString += String(thermocouple1.readInternal());
  dataString += ",";
  
  // read three sensors and append to the string:
  int sensor = analogRead( 0 );
  sensorString = String(sensor);
  dataString += sensorString; 
  dataString += ","; 
  tft.textSetCursor(340, 10);
  //tft.textWrite( sensorString );

  sensor = analogRead( 1 );
  sensorString = String(sensor);
  dataString += sensorString; 
  dataString += ","; 
  tft.textSetCursor(440, 10);
   //tft.textWrite( sensorString );

  sensor = analogRead( 2 );
  sensorString = String(sensor);
  dataString += sensorString; 
  dataString += ","; 
  tft.textSetCursor(540, 10);
  //tft.textWrite( sensorString );

  sensor = analogRead( 3 );
  sensorString = String(sensor);
  dataString += sensorString; 
  dataString += ","; 
  tft.textSetCursor(640, 10);
  //tft.textWrite( sensorString );


  dataFile.println(dataString);

  // print to the serial port too:
  Serial.println(dataString);
  
  // The following line will 'save' the file to the SD card after every
  // line of data - this will use more power and slow down how much data
  // you can read but it's safer! 
  // If you want to speed up the system, remove the call to flush() and it
  // will save the file only every 512 bytes - every time a sector on the 
  // SD card is filled with data.
  dataFile.flush();
  
  delay(LOG_INTERVAL);
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

void startSD() {
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(4)) { // formerly 10,11,12,13 (malformed call?). Value of 4 from datalogger docs
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");
}

void startTFT() {
  Serial.println("Trying to initialize RA8875 though SPI");
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }
  Serial.println("Found RA8875");

  tft.displayOn(true);
  tft.GPIOX(true);                              // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);
  //tft.writeReg(0x22, 0x10); // set text to portrait mode (rotate 90 degrees)
  //tft.writeReg(0x20, 0x04); //reverse scan direction for Y (largest to smallest :))  
    
  Serial.print("Test the TFT display ... ");
  // Something to let us know it is working...
  tft.fillScreen(RA8875_BLACK);
  tft.fillScreen(RA8875_RED);
  tft.fillScreen(RA8875_GREEN);
  tft.fillScreen(RA8875_BLUE);
  tft.fillScreen(RA8875_BLACK);

//  plotArea();
  tft.textMode();
  tft.textTransparent(RA8875_WHITE);
  tft.textEnlarge(0);
  tft.textSetCursor(20, 10);
  tft.textWrite("Chart Recorder v03");

  tft.textSetCursor(300, 10);
  tft.textWrite("Ch 0: ");
  tft.textSetCursor(400, 10);
  tft.textWrite("Ch 1: ");
  tft.textSetCursor(500, 10);
  tft.textWrite("Ch 2: ");
  tft.textSetCursor(600, 10);
  tft.textWrite("Ch 3: ");

 Serial.println("Done!");

}

