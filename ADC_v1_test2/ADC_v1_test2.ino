// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A15;  // Analog input pin that the potentiometer is attached to

int sensorValue = 0;          // value read from A15
float outputValue = 0;        // value output to the PWM (analog out)

unsigned long start = 0;
unsigned long time = 0;

void setup() {
  while (!Serial);     // used for leonardo debugging
 
  Serial.begin(115200);
  Serial.println(F("ADC"));
  Serial.println(F("time (ms),A15 (V)"));

  start = millis()
}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  time = millis() - start;
  // map it to the range of the analog out:
  outputValue = mapfloat(sensorValue, 0, 1023, 0.0, 5.0);

  // print the results to the serial monitor:
  Serial.print(time);
  Serial.print(",");
  Serial.println(outputValue);

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(2);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

