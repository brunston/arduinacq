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
#include "TFTButton.h"

// set up variables TFT utility library functions:
#define RA8875_CS         7   // RA8875 chip select for ISP communication
#define CTP_INT           2    // touch data ready for read from FT5x06 touch controller
#define RA8875_RESET      9    // Adafruit library puts a short low reset pulse at startup on this pin.
                               // Not relevant for TFTM070 according to doc.

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
FT5x06 cmt = FT5x06(CTP_INT);

TFTButton button = TFTButton(100,100,300,100,tft);

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
  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    //while (1) ;
  }
  Serial.println("Trying to initialize RA8875 though SPI");
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  //starting TFT

  Serial.println("Found RA8875");
  cmt.init(false);
  tft.displayOn(true);
  tft.GPIOX(true);                              // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  //visual cue to show that the screen works
  tft.fillScreen(RA8875_BLACK);
  tft.fillScreen(RA8875_RED);
  tft.fillScreen(RA8875_GREEN);
  tft.fillScreen(RA8875_BLUE);
  tft.fillScreen(RA8875_BLACK);

  button.drawButton();

  // basic readout test, just print the current temp
  Serial.print("Internal Temp 0 = ");
  Serial.println(thermocouple0.readInternal());
  Serial.print("Internal Temp 1 = ");
  Serial.println(thermocouple1.readInternal());


  Serial.println("Setup done.");


}

word prev_coordinates[10];
word transfer_coords[10];
byte nr_of_touches = 0;

void loop(){
  byte registers[FT5206_NUMBER_OF_REGISTERS];
  byte prev_nr_of_touches = 0;
  word coordinates[10];

  // ESTABLISH BUTTON COORDINATES
  word SIZE_OF_BUTTONS[2] = {100,50};
  word test_button[4] = {300, 300 + SIZE_OF_BUTTONS[0], 300, 300 + SIZE_OF_BUTTONS[1]};
  if (cmt.touched()){
    cmt.getRegisterInfo(registers);
    nr_of_touches = cmt.getTouchPositions(coordinates,registers);
    prev_nr_of_touches = nr_of_touches;

    //for (byte i = 0 ; i < prev_nr_of_touches; i++){
    //  word x = prev_coordinates[i * 2];
    //  word y = prev_coordinates[i * 2 + 1];
    //  tft.fillCircle(x, y, 70, RA8875_BLACK);
    //}

    for (byte i = 0; i < nr_of_touches; i++){
      word x = coordinates[i*2];
      word y = coordinates[i * 2 + 1];

      Serial.println("coord x = ");
      Serial.println(x);
      Serial.println("coord y = ");
      Serial.println(y);
      //tft.fillCircle(x,y,10,RA8875_WHITE);

      // CHECK FOR BUTTON PRESSES
      withinBounds(x, y, test_button);
    }
    delay(10);
    memcpy(prev_coordinates, coordinates, 20);
    memcpy(transfer_coords, coordinates, 20);
  }

  // DRAW BUTTONS
  button.drawButton();
  tft.graphicsMode();
  tft.fillRect(int(test_button[0]),int(test_button[2]), SIZE_OF_BUTTONS[0], SIZE_OF_BUTTONS[1], RA8875_WHITE);
  tft.graphicsMode();
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
  if (!SD.begin(10,11,12,13)) { // pins connected from SD to Arduino
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");
}
void withinBounds(word x, word y, word bounds[]) { // determines if a touch is within a "button"'s bounds 
 
    if (x > bounds[0] && x < bounds[1] && y > bounds[2] && y < bounds[3]){
      tft.textMode();
      tft.textSetCursor(50,10);
      tft.textEnlarge(0);
      tft.textWrite("Button touched.");
  }
}
