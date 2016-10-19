/***************************************************************************************
  arduinacq data acquisition and chart recording system
  with ER-TFTM070-5 (LCD) from EastRising (bought from buydisplay.com). Depends on RA8875 library from Adafruit.
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
//#include "TFTButton.h"

// set up variables TFT utility library functions:
#define RA8875_CS         7   // RA8875 chip select for ISP communication
#define CTP_INT           2    // touch data ready for read from FT5x06 touch controller
#define RA8875_RESET      9    // Adafruit library puts a short low reset pulse at startup on this pin.
// Not relevant for TFTM070 according to doc.

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
FT5x06 cmt = FT5x06(CTP_INT);

//TFTButton button = TFTButton(100,100,300,100,tft);

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

// Our logging interval in milliseconds, and timing/logging info
#define LOG_INTERVAL 500

unsigned long log_timer;

// BUTTON INITIALIZATION
int BUTTONSIZE[2] = {100, 50};
// int button_name[4] = {leftx, rightx, topy, boty};
int b_start_logging[4] = {20, 20 + BUTTONSIZE[0], 20, 20 + BUTTONSIZE[1]};
int b_stop_logging[4] = {20 + BUTTONSIZE[0] + 10, 20 + 2 * BUTTONSIZE[0] + 10, 20 , 20 + BUTTONSIZE[1]};
int b_incr_time[4] = {130, 130 + BUTTONSIZE[0], 120, 120 + BUTTONSIZE[1]};
int b_decr_time[4] = {130 + BUTTONSIZE[0] + 10, 130 + 2 * BUTTONSIZE[0] + 10, 120 , 120 + BUTTONSIZE[1]};
int b_incr_temp[4] = {130, 130 + BUTTONSIZE[0], 220, 220 + BUTTONSIZE[1]};
int b_decr_temp[4] = {130 + BUTTONSIZE[0] + 10, 130 + 2 * BUTTONSIZE[0] + 10, 220 , 220 + BUTTONSIZE[1]};
int b_plot_mean[4] = {130, 130 + BUTTONSIZE[0], 320, 320 + BUTTONSIZE[1]};
int b_plot_mxmn[4] = {130 + BUTTONSIZE[0] + 10, 130 + 2 * BUTTONSIZE[0] + 10, 320 , 320 + BUTTONSIZE[1]};
int b_plot_inst[4] = {130 + 2*BUTTONSIZE[0] + 20, 130 + 3 * BUTTONSIZE[0] + 10, 320 , 320 + BUTTONSIZE[1]};

// BUTTON STATUS
bool b_start_logging_status = false;
bool b_stop_logging_status = false;
bool b_incr_time_status = false;
bool b_decr_time_status = false;
bool b_incr_temp_status = false;
bool b_decr_temp_status = false;
bool b_plot_mean_status = false;
bool b_plot_mxmn_status = false;
bool b_plot_inst_status = false;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  //starting TFT
  Serial.println("Trying to initialize RA8875 though SPI");
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  Serial.println("Found RA8875");
  cmt.init(false);
  tft.displayOn(true);
  tft.GPIOX(true);                              // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  // SCREEN STARTUP
  tft.fillScreen(RA8875_BLACK);
  tft.fillScreen(RA8875_RED);
  tft.fillScreen(RA8875_GREEN);
  tft.fillScreen(RA8875_BLUE);
  tft.fillScreen(RA8875_BLACK);

  startRTC();
  startSD();

  // clear message area
  tft.graphicsMode();
  tft.fillRect(500, 20, 300, 20, RA8875_BLACK);

  // Open up the file we're going to log to!
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  if (! dataFile) {
    Serial.println("error opening datalog.csv");
    // Wait forever since we cant write data
    //while (1) ;
  }
  dataFile.close();

  // DRAW GUI
  drawButton(b_start_logging, "start log");
  drawButton(b_stop_logging, "stop log");
  drawButton(b_incr_time, "time (+ 1hr)");
  drawButton(b_decr_time, "time (- 1hr)");
  drawButton(b_incr_temp, "temp (+ 20c)");
  drawButton(b_decr_temp, "temp (- 20c)");
  drawButton(b_plot_mean, "mean plot");
  drawButton(b_plot_mxmn, "mxmn plot");
  drawButton(b_plot_inst, "inst plot");

  // basic readout test, just print the current temp
  Serial.print("Internal Temp 0 = ");
  Serial.println(thermocouple0.readInternal());
  Serial.print("Internal Temp 1 = ");
  Serial.println(thermocouple1.readInternal());

  // GET TIMER LOG
  log_timer = millis();
  Serial.println("Setup done.");

}

word prev_coordinates[10];
word transfer_coords[10];
byte nr_of_touches = 0;



// status of button pushed and gui status
bool logging_status = false;
bool init_screen = true;
int record_interval = 500;

// array of maxmin data for graph resizing REMOVE? POST-161019 GUI DISCUSSION
int resize_data[41][6]; // we want to store each of these maxmins for our whole graph width (16*41)
int every_16[16][6]; // we want to get max min of every 16 data point chunk

void loop() {
  byte registers[FT5206_NUMBER_OF_REGISTERS];
  byte prev_nr_of_touches = 0;
  word coordinates[10];
  String data = "";
  // HANDLE TOUCH EVENTS
  if (cmt.touched()) {
    cmt.getRegisterInfo(registers);
    nr_of_touches = cmt.getTouchPositions(coordinates, registers);
    prev_nr_of_touches = nr_of_touches;

    //for (byte i = 0 ; i < prev_nr_of_touches; i++){
    //  word x = prev_coordinates[i * 2];
    //  word y = prev_coordinates[i * 2 + 1];
    //  tft.fillCircle(x, y, 70, RA8875_BLACK);
    //}

    for (byte i = 0; i < nr_of_touches; i++) {
      word x = coordinates[i * 2];
      word y = coordinates[i * 2 + 1];
      //DEBUG
      Serial.println("coord x = ");
      Serial.println(x);
      Serial.println("coord y = ");
      Serial.println(y);
      //tft.fillCircle(x,y,10,RA8875_WHITE);

      // CHECK FOR BUTTON PRESSES, LOGGING STATUS
      b_start_logging_status = withinBounds(x, y, b_start_logging);
      b_stop_logging_status = withinBounds(x, y, b_stop_logging);
      
      if (logging_status == false && b_start_logging_status == true) {
        logging_status = true;
        init_screen == false;
        makeGraph();
        Serial.println("logging status true");//DEBUG
      }
      else if (logging_status == true && b_stop_logging_status == true) {
        logging_status = false;
        Serial.println("logging status false");//DEBUG
      }
      if (init_screen) {
        b_incr_time_status = withinBounds(x, y, b_incr_time);
        b_decr_time_status = withinBounds(x, y, b_decr_time);
        b_incr_temp_status = withinBounds(x, y, b_incr_temp);
        b_decr_time_status = withinBounds(x, y, b_decr_temp);
        b_plot_mean_status = withinBounds(x, y, b_plot_mean);
        b_plot_mxmn_status = withinBounds(x, y, b_plot_mxmn);
        b_plot_inst_status = withinBounds(x, y, b_plot_inst);
      }
    }

    delay(10);
    memcpy(prev_coordinates, coordinates, 20);
    memcpy(transfer_coords, coordinates, 20);
  }

  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (init_screen) {
    tft.textMode();
    tft.textSetCursor(350, 10);
    tft.textEnlarge(0);
    tft.textColor(RA8875_BLACK, RA8875_WHITE);
    tft.textWrite("arduinacq");
    tft.textSetCursor(360, 100);
    tft.textColor(RA8875_BLACK, RA8875_WHITE);
    tft.textWrite("INITSCR");
  }
  if (logging_status) {
    updateStatus("Logging running.        ");
    if ((millis() - log_timer) >= LOG_INTERVAL) {
      DateTime now = RTC.now();
      float d_vals[6] = {analogRead(A0), analogRead(A1), analogRead(A2), analogRead(A3), thermocouple0.readInternal(), thermocouple1.readInternal()};
      Serial.println("attempting to write to log"); //DEBUG
      dataFile.print(now.year(), DEC);
      dataFile.print(now.month(), DEC);
      dataFile.print(now.day(), DEC);
      dataFile.print("\t");
      dataFile.print(now.hour(), DEC);
      dataFile.print(':');
      dataFile.print(now.minute(), DEC);
      dataFile.print(':');
      dataFile.print(now.second(), DEC);
      dataFile.print("\t");
      dataFile.print(d_vals[0] * 4.883); // conversion from 0-1024 value to mV
      dataFile.print("\t");
      dataFile.print(d_vals[1] * 4.883);
      dataFile.print("\t");
      dataFile.print(d_vals[2] * 4.883);
      dataFile.print("\t");
      dataFile.print(d_vals[3] * 4.883);
      dataFile.print("\t");
      dataFile.print(d_vals[4]);
      dataFile.print("\t");
      dataFile.println(d_vals[5]);
      if ((millis() - log_timer) >= record_interval) {
        updateGraph(d_vals[0], d_vals[1], d_vals[2], d_vals[3], d_vals[4], d_vals[5]);
      }
      log_timer = millis();
    }
    dataFile.close();
  }
  else {
    tft.graphicsMode();
    updateStatus("Logging stopped.        ");
    dataFile.close();
  }

}

// RTC AND SD INITIALIZATION FUNCTIONS
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
  if (!SD.begin(10, 11, 12, 13)) { // pins connected from SD to Arduino
    Serial.println("Card failed, or not present");
    updateStatus("No SD card. Insert and reboot.");
    // don't do anything more:
    while (1) ;
  }
  else {
    Serial.println("card initialized.");
    updateStatus("SD card initialized.    ");
  }
}

// GUI FUNCTIONS
void updateStatus(char update_cond[]) {
  tft.textMode();
  tft.textSetCursor(500, 20);
  tft.textColor(RA8875_WHITE, RA8875_BLACK);
  tft.textWrite(update_cond);
}

bool withinBounds(int x, int y, int button[4]) { // determines if a touch is within a "button"'s bound

  if (x > button[0] && x < button[1] && y > button[2] && y < button[3]) {
    return true;
  }
  else {
    return false;
  }
}

int graphCursorX = 101; // change each time we write a new pixel of data.
void updateGraph(float dat_a0, float dat_a1, float dat_a2, float dat_a3, float dat_t0, float dat_t1) {
  if (graphCursorX == 750) {
    while (1); //temporary while I implement the rest of this functionality.
  }
  tft.graphicsMode();
  // 450 - is because screen is upper-left 0,0 indexed;
  // * 4.883 is mV per analogRead unit; * 3.5 is pixels per degree C;
  // * 0.07 is pixels per mV; + 0.5 is for rounding; .
  tft.drawPixel(graphCursorX, int(450 - dat_a0 * 4.883 * 0.07 + 0.5), RA8875_WHITE);
  tft.drawPixel(graphCursorX, int(450 - dat_a1 * 4.883 * 0.07 + 0.5), RA8875_YELLOW);
  tft.drawPixel(graphCursorX, int(450 - dat_a2 * 4.883 * 0.07 + 0.5), RA8875_GREEN);
  tft.drawPixel(graphCursorX, int(450 - dat_a3 * 4.883 * 0.07 + 0.5), RA8875_CYAN);
  tft.drawPixel(graphCursorX, int(450 - dat_t0 * 3.5 + 0.5), RA8875_RED);
  tft.drawPixel(graphCursorX, int(450 - dat_t1 * 3.5 + 0.5), RA8875_MAGENTA);
  graphCursorX = graphCursorX + 1;
}

void horizShrinkGraph() { // when we reach the limit of our graph display, shift everything over.
}

void drawButton(int button[4], char strarr[]) {
  tft.graphicsMode();
  tft.fillRect(button[0], button[2], button[1] - button[0], button[3] - button[2], RA8875_WHITE);
  tft.textMode();
  tft.textSetCursor(button[0], button[2]);
  tft.textEnlarge(0);
  tft.textColor(RA8875_BLACK, RA8875_WHITE);
  tft.textWrite(strarr);
}

void makeGraph() {
  // clear graph area, reset graphCursor
  tft.fillRect(100, 100, 650, 350, RA8875_BLACK);
  graphCursorX = 101;

  tft.textMode();
  tft.textSetCursor(350, 10);
  tft.textEnlarge(0);
  tft.textColor(RA8875_BLACK, RA8875_WHITE);
  tft.textWrite("arduinacq");

  tft.textSetCursor(310, 30);
  tft.textColor(RA8875_WHITE, RA8875_BLACK);
  tft.textWrite("Color Legend");
  tft.textSetCursor(310, 50);
  tft.textEnlarge(0);
  tft.textColor(RA8875_WHITE, RA8875_BLACK);
  tft.textWrite("A0");
  tft.textSetCursor(330, 50);
  tft.textEnlarge(0);
  tft.textColor(RA8875_YELLOW, RA8875_BLACK);
  tft.textWrite("A1");
  tft.textSetCursor(350, 50);
  tft.textEnlarge(0);
  tft.textColor(RA8875_GREEN, RA8875_BLACK);
  tft.textWrite("A2");
  tft.textSetCursor(370, 50);
  tft.textEnlarge(0);
  tft.textColor(RA8875_CYAN, RA8875_BLACK);
  tft.textWrite("A3");
  tft.textSetCursor(390, 50);
  tft.textEnlarge(0);
  tft.textColor(RA8875_RED, RA8875_BLACK);
  tft.textWrite("T0");
  tft.textSetCursor(410, 50);
  tft.textEnlarge(0);
  tft.textColor(RA8875_MAGENTA, RA8875_BLACK);
  tft.textWrite("T1");

  tft.textSetCursor(20, 80);
  tft.textEnlarge(0);
  tft.textColor(RA8875_BLACK, RA8875_WHITE);
  tft.textWrite("Volt.");
  tft.textColor(RA8875_WHITE, RA8875_BLACK);
  tft.textSetCursor(20, 100);
  tft.textWrite("5000[mV]");
  tft.textSetCursor(20, 170);
  tft.textWrite("4000[mV]");
  tft.textSetCursor(20, 240);
  tft.textWrite("3000[mV]");
  tft.textSetCursor(20, 310);
  tft.textWrite("2000[mV]");
  tft.textSetCursor(20, 380);
  tft.textWrite("1000[mV]");
  tft.textSetCursor(20, 440);
  tft.textWrite("0[mV]");
  tft.textSetCursor(751, 80);
  tft.textColor(RA8875_BLACK, RA8875_WHITE);
  tft.textWrite("Temp.");
  tft.textColor(RA8875_WHITE, RA8875_BLACK);
  tft.textSetCursor(761, 100);
  tft.textWrite("100c");
  tft.textSetCursor(761, 170);
  tft.textWrite("80 c");
  tft.textSetCursor(761, 240);
  tft.textWrite("60 c");
  tft.textSetCursor(761, 310);
  tft.textWrite("40 c");
  tft.textSetCursor(761, 380);
  tft.textWrite("20 c");
  tft.textSetCursor(761, 440);
  tft.textWrite("0 c");

  tft.graphicsMode();
  tft.drawLine(100, 450, 750, 450, RA8875_WHITE);
  tft.drawLine(100, 100, 100, 450, RA8875_WHITE);

  tft.drawLine(90, 100, 100, 100, RA8875_WHITE);
  tft.drawLine(90, 170, 100, 170, RA8875_WHITE);
  tft.drawLine(90, 240, 100, 240, RA8875_WHITE);
  tft.drawLine(90, 310, 100, 310, RA8875_WHITE);
  tft.drawLine(90, 380, 100, 380, RA8875_WHITE);
  tft.drawLine(90, 450, 100, 450, RA8875_WHITE);

  tft.drawLine(750, 100, 760, 100, RA8875_WHITE);
  tft.drawLine(750, 170, 760, 170, RA8875_WHITE);
  tft.drawLine(750, 240, 760, 240, RA8875_WHITE);
  tft.drawLine(750, 310, 760, 310, RA8875_WHITE);
  tft.drawLine(750, 380, 760, 380, RA8875_WHITE);
  tft.drawLine(750, 450, 760, 450, RA8875_WHITE);

}
