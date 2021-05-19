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

#define PIN            0
#define NUMPIXELS      8
cRGB value;
WS2812 LED(NUMPIXELS); // 1 LED

byte ledCounter = 0;
int colors = 0;
unsigned long previousMillis = 0;
const long interval = 1;
bool updownCount = false;
byte simonSays[255]; //ATTENTION if you set this array to high count then the game will crash!
int simonCount = 0;


void ledSetAll(byte r, byte g, byte b) {
  value.r = r; value.g = g; value.b = b;
  for (byte i = 0; i < NUMPIXELS; i++) {
    LED.set_crgb_at(i, value); // Set value at LED found at index 0
  }
}

void ledSet(byte led, byte r, byte g, byte b) {
  value.r = r; value.g = g; value.b = b;
  LED.set_crgb_at(led, value); // Set value at LED found at index 0
}

void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  analogReference(DEFAULT);

  LED.setOutput(PIN);
  ledSet(0, 0, 0, 255);
  ledSet(1, 255, 0, 0);
  ledSet(2, 255, 255, 0);
  ledSet(3, 0, 255, 0);
  ledSet(4, 0, 255, 0);
  ledSet(5, 0, 255, 0);
  ledSet(6, 0, 255, 0);
  ledSet(7, 0, 255, 0);
  LED.sync(); // Sends the value to the LED
  delay(1000);
}

void loop()
{
  simonSays[simonCount++] = random(4);

  //show sequence
  for (int i = 0; i < simonCount; i++) {
    ledSetAll(0, 0, 0);
    LED.sync(); // Sends the value to the LED
    delay(600);
    if (simonSays[i] == 0) {
      ledSet(1, 255, 0, 0);
    }
    if (simonSays[i] == 1) {
      ledSet(0, 0, 0, 255);
    }
    if (simonSays[i] == 2) {
      ledSet(2, 255, 255, 0);
    }
    if (simonSays[i] == 3) {
      ledSet(3, 0, 255, 0);
    }
    LED.sync(); // Sends the value to the LED
    delay(600);
  }
  ledSetAll(0, 0, 0);
  ledSet(1, 255, 0, 0);
  ledSet(0, 0, 0, 255);
  ledSet(2, 255, 255, 0);
  ledSet(3, 0, 255, 0);
  LED.sync(); // Sends the value to the LED

  //do sequence
  bool rightSequence = false;
  for (int i = 0; i < simonCount; i++) {
    if (getButton() == simonSays[i]) {
      rightSequence = true;
    } else {
      rightSequence = false;
      break;
    }
  }
  delay(600);
  ledSetAll(0, 0, 0);
  if (rightSequence) {
    ledSet(4, 0, 255, 0);
    ledSet(5, 0, 255, 0);
    ledSet(6, 0, 255, 0);
    ledSet(7, 0, 255, 0);
  } else {
    ledSet(4, 255, 0, 0);
    ledSet(5, 255, 0, 0);
    ledSet(6, 255, 0, 0);
    ledSet(7, 255, 0, 0);
    simonCount = 0;
  }
  LED.sync(); // Sends the value to the LED
  delay(1000);
}


byte getButton() {
  randomSeed(millis());
  while (true) {
    if (readKey() == 1) {
      ledSet(1, 0, 0, 0);
      LED.sync(); // Sends the value to the LED
      while (readKey() == 1) {
        delay(20);
      }
      ledSet(1, 255, 0, 0);
      LED.sync(); // Sends the value to the LED
      return 0;
    }
    if (readKey() == 2) {
      ledSet(0, 0, 0, 0);
      LED.sync(); // Sends the value to the LED
      while (readKey() == 2) {
        delay(20);
      }
      ledSet(0, 0, 0, 255);
      LED.sync(); // Sends the value to the LED
      return 1;
    }
    if (readKey() == 3) {
      ledSet(2, 0, 0, 0);
      LED.sync(); // Sends the value to the LED
      while (readKey() == 3) {
        delay(20);
      }
      ledSet(2, 255, 255, 0);
      LED.sync(); // Sends the value to the LED
      return 2;
    }
    if (readKey() == 4) {
      ledSet(3, 0, 0, 0);
      LED.sync(); // Sends the value to the LED
      while (readKey() == 4) {
        delay(20);
      }
      ledSet(3, 0, 255, 0);
      LED.sync(); // Sends the value to the LED
      return 3;
    }
  }
}


int readADC() {
  int avrADC = 0;
  analogRead(A1);
  for (byte i = 0; i < 5; ++i)
  {
    avrADC += analogRead(A1);
  }
  avrADC /= 5;
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
    if (sameADCCount > 3)
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
