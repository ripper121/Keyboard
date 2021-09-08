#include "TrinketHidCombo.h"
#include <WS2812.h>

#define S1 906
#define S2 708
#define S3 462
#define S4 235
#define S5 870
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
const float brightness = 0.1;
WS2812 LED(NUMPIXELS); // 1 LED

#define KEY_NOKEY             0x00
byte lastKey, key = KEY_NOKEY;

void ledSet(byte r, byte g, byte b, float brightness) {
  value.r = r * brightness; value.g = g * brightness; value.b = b * brightness;
  for (byte i = 0; i < NUMPIXELS; i++) {
    LED.set_crgb_at(i, value); // Set value at LED found at index 0
  }
  LED.sync(); // Sends the value to the LED
}

void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  analogReference(DEFAULT);

  LED.setOutput(PIN); // Digital Pin 9
  ledSet(255, 64, 0, brightness);

  TrinketHidCombo.begin();
}

void loop() {
  if (TrinketHidCombo.isConnected() != 0) {
    key = readKey();
    if (key != KEY_NOKEY) {
      if ((key == KEYCODE_ARROW_UP || key == KEYCODE_ARROW_DOWN) && lastKey == key) {
        if (key == MMKEY_VOL_DOWN || key == MMKEY_VOL_UP) {
          TrinketHidCombo.pressMultimediaKey(key);
        } else {
          TrinketHidCombo.pressKey(0, key);
        }
        TrinketHidCombo.pressKey(0, 0);
      }
      else  if (lastKey != key) {
        if (key == MMKEY_VOL_DOWN || key == MMKEY_VOL_UP) {
          TrinketHidCombo.pressMultimediaKey(key);
        } else {
          TrinketHidCombo.pressKey(0, key);
        }
        TrinketHidCombo.pressKey(0, 0);
      }
      digitalWrite(1, !digitalRead(1));
    }
    lastKey = key;
  }
  TrinketHidCombo.poll();
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
      return KEY_NOKEY;
    case S1-RANGE ... S1+RANGE:
      return KEYCODE_ARROW_UP;
    case S2-RANGE ... S2+RANGE:
      return KEYCODE_W;
    case S8-RANGE ... S8+RANGE:
      return KEYCODE_D;
    case S4-RANGE ... S4+RANGE:
      return KEYCODE_S;
    case S6-RANGE ... S6+RANGE:
      return KEYCODE_F5;
    case S3-RANGE ... S3+RANGE:
      return KEYCODE_ARROW_DOWN;
    case S5-RANGE ... S5+RANGE:
      return KEYCODE_F;
    case S7-RANGE ... S7+RANGE:
      return KEYCODE_A;
    case RR-RANGE ... RR+RANGE:
      return KEYCODE_L;
    case RM-RANGE ... RM+RANGE:
      return KEYCODE_P;
    case RL-RANGE ... RL+RANGE:
      return KEYCODE_J;
    case LL-RANGE ... LL+RANGE:
      return MMKEY_VOL_DOWN;
    case LR-RANGE ... LR+RANGE:
      return MMKEY_VOL_UP;
    case LM-RANGE ... LM+RANGE:
      return KEYCODE_Q;
    default:
      return KEY_NOKEY;
  }

  return  KEY_NOKEY;
}
