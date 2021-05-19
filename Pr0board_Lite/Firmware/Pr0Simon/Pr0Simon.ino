#include <WS2812.h>
#include <EEPROM.h>
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
#define SHOW_DELAY     300

byte simonSays[255]; //ATTENTION if you set this array to high count then the game will crash!
byte simonCount = 0;

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
      ledSet(1, 0, 255, 0);
      break;
    case 2:
      ledSet(0, 255, 0, 0);
      break;
    case 3:
      ledSet(2, 0, 0, 255);
      break;
    case 4:
      ledSet(3, 255, 255, 0);
      break;
    case 5:
      ledSet(4, 0, 255, 255);
      break;
    case 6:
      ledSet(5, 0, 255, 255);
      break;
    case 7:
      ledSet(7, 0, 255, 255);
      break;
    case 8:
      ledSet(6, 0, 255, 255);
      break;
  }
  LED.sync();
}

void clearButton(byte num) {
  switch (num) {
    case 1:
      ledSet(1, 0, 0, 0);
      break;
    case 2:
      ledSet(0, 0, 0, 0);
      break;
    case 3:
      ledSet(2, 0, 0, 0);
      break;
    case 4:
      ledSet(3, 0, 0, 0);
      break;
    case 5:
      ledSet(4, 0, 0, 0);
      break;
    case 6:
      ledSet(5, 0, 0, 0);
      break;
    case 7:
      ledSet(7, 0, 0, 0);
      break;
    case 8:
      ledSet(6, 0, 0, 0);
      break;
  }
  LED.sync();
}

byte getButton() {
  //randomSeed(millis());
  while (true) {
    byte rKey = readKey();
    clearButton(rKey); //set led for pressed button
    //wait for Key release
    while (readKey() == rKey) {
      delay(20);
    }
    setButton(rKey); //set led for pressed button
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

void showSequence() {
  //show sequence
  for (int i = 0; i < simonCount; i++) {
    if (simonSays[i] > 4)
      return;
    ledSetAll(0, 0, 0);
    LED.sync(); // Sends the value to the LED
    delay(SHOW_DELAY);
    setButton(simonSays[i]);
    delay(SHOW_DELAY);
  }
  delay(SHOW_DELAY);
}


void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  analogReference(DEFAULT);

  LED.setOutput(PIN);
  ledSetAll(100, 100, 100);
  ledSet(6, 255, 0, 0);
  ledSet(7, 0, 255, 0);
  LED.sync();
  byte gButton = 0;
  while (gButton == 0) {
    gButton = getButton();
  }
  if (gButton == 7) {
    simonCount = EEPROM.read(0);
    EEPROM.get(1, simonSays);
    showSequence();
    simonCount = 0;
  }
  if (gButton == 8) {
    ledSetAll(255, 0, 0);
    LED.sync();
    EEPROM.write(0, 0);
    for (int i = 1 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 255);
    }
    ledSetAll(0, 255, 0);
    LED.sync();
  }
  delay(1000);
  ledSet(1, 0, 255, 0);
  ledSet(0, 255, 0, 0);
  ledSet(2, 0, 0, 255);
  ledSet(3, 255, 255, 0);
  ledSet(4, 0, 255, 0);
  ledSet(5, 0, 255, 0);
  ledSet(6, 0, 255, 0);
  ledSet(7, 0, 255, 0);
  LED.sync(); // Sends the value to the LED
  delay(1000);
}


void loop()
{
  randomSeed(millis());
  simonSays[simonCount++] = (random(4) + 1);

  showSequence();

  ledSetAll(0, 0, 0);
  ledSet(1, 0, 255, 0);
  ledSet(0, 255, 0, 0);
  ledSet(2, 0, 0, 255);
  ledSet(3, 255, 255, 0);
  LED.sync(); // Sends the value to the LED

  //do sequence
  bool rightSequence = false;
  for (int i = 0; i < simonCount; i++) {
    byte gButton = 0;
    while ((gButton == 0) || (gButton > 4 )) {
      gButton = getButton();
    }
    if (gButton == simonSays[i]) {
      rightSequence = true;
    } else {
      rightSequence = false;
      break;
    }
  }
  delay(SHOW_DELAY * 2);

  ledSetAll(0, 0, 0);
  if (rightSequence) {
    ledSet(4, 0, 255, 0);
    ledSet(5, 0, 255, 0);
    ledSet(6, 0, 255, 0);
    ledSet(7, 0, 255, 0);
    //write game Score
    if (simonCount >= EEPROM.read(0)) {
      EEPROM.write(0, simonCount);
      EEPROM.put(1, simonSays);
    }
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
