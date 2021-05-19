#include <WS2812.h>

/*
  Led Numbers
  1 0  4 5
  2 3  7 6
*/

/*
  Key Numbers
  1 2  5 6
  3 4  7 8
*/

//you can get these values from the Pr0boardADC sample
#define S1 906
#define S2 708
#define S3 462
#define S4 235
#define S5 880
#define S6 658
#define S7 406
#define S8 196
#define LL 127
#define LM 289
#define LR 532
#define RL 158
#define RM 342
#define RR 593
#define RANGE 10 //Min diff between 2 ADC values is 26 so range should be less than half of that

//too many checks will slowdown the button readout, but more will be more reliable
#define SAME_KEY_READ_COUNT 2 //only when 2 times the same key was read back it will be accepted
#define ADC_READ_COUNT 2 //average of adc readbacks

//WS2812B first led Pin and Led Count
#define PIN            0
#define NUMPIXELS      8

WS2812 LED(NUMPIXELS);

void ledSetAll(byte r, byte g, byte b) {
  cRGB value;
  value.r = r; value.g = g; value.b = b;
  for (byte i = 0; i < NUMPIXELS; i++) {
    LED.set_crgb_at(i, value); // Set value at LED found at index 0
  }
}

void ledSet(byte led, byte r, byte g, byte b) {
  cRGB value;
  value.r = r; value.g = g; value.b = b;
  LED.set_crgb_at(led, value); // Set value at LED found at index 0
}

void setButton(byte num) {
  switch (num) {
    case 1:
      ledSet(1, 238, 77, 46);
      break;
    case 2:
      ledSet(0, 29, 185, 146);
      break;
    case 3:
      ledSet(2, 176, 173, 5);
      break;
    case 4:
      ledSet(3, 255, 0, 130);
      break;
    case 5:
      ledSet(4, 108, 67, 43);
      break;
    case 6:
      ledSet(5, 255, 8, 233);
      break;
    case 7:
      ledSet(7, 108, 67, 43);
      break;
    case 8:
      ledSet(6, 255, 0, 130);
      break;
    default:
      ledSetAll(0, 0, 0);
      break;
  }
  LED.sync();
}

byte getButton() {
  //randomSeed(millis());
  while (true) {
    byte rKey = readKey();
    setButton(rKey); //set led for pressed button
    //wait for Key release
    while (readKey() == rKey) {
      delay(20);
    }
    setButton(-1); //clear Leds
    return rKey;
  }
}

int readADC() {
  int avrADC = 0;
  for (byte i = 0; i < ADC_READ_COUNT; ++i)
  {
    avrADC += analogRead(A1);
  }
  avrADC /= ADC_READ_COUNT;
  return avrADC;
}


byte readKey() {
  int lastADC, nowADC = -1;
  byte sameADCCount = 0;

  while (true) {
    lastADC = readADC();
    nowADC = readADC();
    if (nowADC == lastADC)
      sameADCCount++;
    else
      sameADCCount = 0;
    if (sameADCCount > SAME_KEY_READ_COUNT)
      break;
  }

  switch (nowADC) {
    case 0 ... RANGE:
      return 0;
    case S1-RANGE ... S1+RANGE:
      return 1;
    case S2-RANGE ... S2+RANGE:
      return 2;
    case S8-RANGE ... S8+RANGE:
      return 8;
    case S4-RANGE ... S4+RANGE:
      return 4;
    case S6-RANGE ... S6+RANGE:
      return 6;
    case S3-RANGE ... S3+RANGE:
      return 3;
    case S5-RANGE ... S5+RANGE:
      return 5;
    case S7-RANGE ... S7+RANGE:
      return 7;
    default:
      return 0;
  }
  return  0;
}


void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  analogReference(DEFAULT);

  LED.setOutput(PIN);
  ledSetAll(0, 0, 0);
  LED.sync(); // Sends the value to the LED
  delay(1000);
}

void loop()
{
  getButton();
}
