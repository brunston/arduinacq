
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// For the screen position
#define FONT_HEIGHT 10  // screen.setTextSize(size); size 1 = 10 pixels, size 2 =20 pixels, and so on.
#define X_LEFT 20
#define X_RIGHT 310
#define Y_TOP 10
#define Y_BOT 220

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Pin name constants.
const int analogInPin = A15;

// Variables
float sensorValue = 0.;
const int num = X_RIGHT - X_LEFT;
int index = 0;
int yPos[num] = {0};
int xPos = 0;

unsigned long tot_start = 0;
unsigned long tot_time = 0;
unsigned long avg_start = 0;
unsigned long avg_time = 0;
float reads = 0.;

void setup() {
  while (!Serial);     // used for leonardo debugging
 
  Serial.begin(9600);
  Serial.println(F("ADC"));

  tft.begin();

  // Something to let us know it is working...
  tft.fillScreen(ILI9341_BLACK);
  tft.fillScreen(ILI9341_RED);
  tft.fillScreen(ILI9341_GREEN);
  tft.fillScreen(ILI9341_BLUE);
  tft.fillScreen(ILI9341_BLACK);

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

  tft.setRotation(1);
  testText();
  
  Serial.println(F("Done!"));
  tot_start = millis();

}

void loop() {
  // read the analog in value and get time:
  avg_start = millis();
  reads = 0.;
  sensorValue = 0.;
  avg_time = 0;
  tot_time = millis() - tot_start;
  while ( avg_time <= 500 ) {
    sensorValue += analogRead(analogInPin);
    avg_time = millis() - avg_start;
    reads++;

    // wait >= 2 milliseconds before the next loop
    // for the analog-to-digital converter to settle
    // after the last reading:
    delay(2);
  }
  sensorValue = sensorValue / reads;
  
  // Delete previous point
  xPos = index + X_LEFT + 1;
  tft.drawPixel(xPos, yPos[index], ILI9341_BLACK);
  
  // map it to the range of the screen:
  yPos[index] = mapfloat(sensorValue, 0, 1023, Y_BOT, Y_TOP);
  tft.drawPixel(xPos, yPos[index], ILI9341_CYAN);

  // write the values to the serial port
  Serial.print(tot_time);
  Serial.print("\t");
  Serial.print(xPos);
  Serial.print("\t");
  Serial.println(yPos[index]);

  // wrap the plot
  if (xPos >= X_RIGHT) {
    index = 0;
    //testText();
  } else {
    // increment the horizontal position:
    index++;
  }
}

unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);

  tft.drawFastVLine(X_LEFT,Y_TOP,(Y_BOT-Y_TOP),ILI9341_WHITE);
  tft.drawFastVLine(X_RIGHT,Y_TOP,(Y_BOT-Y_TOP),ILI9341_WHITE);
  tft.drawFastHLine(X_LEFT,Y_TOP,(X_RIGHT-X_LEFT+1),ILI9341_WHITE);
  tft.drawFastHLine(X_LEFT,Y_BOT,(X_RIGHT-X_LEFT+1),ILI9341_WHITE);
  
  tft.setCursor(5, Y_TOP);
  tft.println("5V");
  tft.setCursor(5, Y_BOT);
  tft.println("0V");
  tft.setCursor(160, Y_BOT+5);
  tft.println("Time");

  return micros() - start;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

