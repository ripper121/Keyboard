#include "HID-Project.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif


#define LEDYELLOW   0
#define LEDGREEN    1
#define KEY_S_2     2
#define KEY_S_1     3
#define S11         4
#define ST1         5
#define S21         6
#define KEY_S_5     7
#define KEY_S_6     8
#define LEDPWM      9
#define WS2812BDI   10
#define S12         MOSI
#define ST2         MISO
#define S22         SCK
#define KEY_S_7     A0
#define KEY_S_8     A1
#define KEY_S_4     A2
#define KEY_S_3     A3
#define DEBOUNCE    50

Adafruit_NeoPixel ws2812b = Adafruit_NeoPixel(8, WS2812BDI, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  pinMode(LEDYELLOW, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);

  pinMode(KEY_S_1, INPUT_PULLUP);
  pinMode(KEY_S_2, INPUT_PULLUP);
  pinMode(KEY_S_3, INPUT_PULLUP);
  pinMode(KEY_S_4, INPUT_PULLUP);
  pinMode(KEY_S_5, INPUT_PULLUP);
  pinMode(KEY_S_6, INPUT_PULLUP);
  pinMode(KEY_S_7, INPUT_PULLUP);
  pinMode(KEY_S_8, INPUT_PULLUP);

  pinMode(S11, INPUT_PULLUP);
  pinMode(ST1, INPUT_PULLUP);
  pinMode(S21, INPUT_PULLUP);

  pinMode(S12, INPUT_PULLUP);
  pinMode(ST2, INPUT_PULLUP);
  pinMode(S22, INPUT_PULLUP);

  BootKeyboard.begin();
  Keyboard.begin();
  Consumer.begin();

  ws2812b.begin();
  ws2812b.setBrightness(100);
  ws2812b.show(); // Initialize all pixels to 'off'
}

byte ledCounter = 0;
int colors = 0;
unsigned long previousMillis = 0;
const long interval = 1;
bool updownCount = false;

void loop()
{
  unsigned long currentMillis = millis();

  if (BootKeyboard.getLeds() & LED_CAPS_LOCK) {
    //Serial.println("LED_CAPS_LOCK");
    digitalWrite(LEDYELLOW, HIGH);
  }
  else {
    digitalWrite(LEDYELLOW, LOW);
  }
  if (BootKeyboard.getLeds() & LED_NUM_LOCK) {
    //Serial.println("LED_NUM_LOCK");
    digitalWrite(LEDGREEN, HIGH);
  }
  else {
    digitalWrite(LEDGREEN, LOW);
  }


  if (!digitalRead(S21)) {
    Serial.println("S21");
    Consumer.write(MEDIA_VOLUME_DOWN);
    delay(DEBOUNCE);
    while (!digitalRead(S21)) {}
    Consumer.release(MEDIA_VOLUME_DOWN);
  }
  if (!digitalRead(ST1)) {
    Serial.println("ST1");
    Consumer.write(MEDIA_VOLUME_MUTE);
    delay(DEBOUNCE);
    while (!digitalRead(ST1)) {}
    Consumer.release(MEDIA_VOLUME_MUTE);
  }
  if (!digitalRead(S11)) {
    Serial.println("S11");
    Consumer.press(MEDIA_VOLUME_UP);
    delay(DEBOUNCE);
    while (!digitalRead(S11)) {}
    Consumer.release(MEDIA_VOLUME_UP);
  }


  if (!digitalRead(S22)) {
    Serial.println("S22");
    Consumer.press(MEDIA_PREV);
    delay(DEBOUNCE);
    while (!digitalRead(S22)) {}
    Consumer.release(MEDIA_PREV);
  }
  if (!digitalRead(ST2)) {
    Serial.println("ST2");
    Consumer.press(MEDIA_PLAY_PAUSE);
    delay(DEBOUNCE);
    while (!digitalRead(ST2)) {}
    Consumer.release(MEDIA_PLAY_PAUSE);
  }
  if (!digitalRead(S12)) {
    Serial.println("S12");
    Consumer.press(MEDIA_NEXT);
    delay(DEBOUNCE);
    while (!digitalRead(S12)) {}
    Consumer.release(MEDIA_NEXT);
  }


  if (!digitalRead(KEY_S_5)) {
    Serial.println("KEY_S_5");
    Keyboard.press(KEY_LEFT);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_5)) {}
    Keyboard.release(KEY_LEFT);
  }

  if (!digitalRead(KEY_S_6)) {
    Serial.println("KEY_S_6");
    Keyboard.press(KEY_DOWN);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_6)) {}
    Keyboard.release(KEY_DOWN);
  }
  if (!digitalRead(KEY_S_7)) {
    Serial.println("KEY_S_7");
    Keyboard.press(KEY_RIGHT);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_7)) {}
    Keyboard.release(KEY_RIGHT);
  }
  if (!digitalRead(KEY_S_8)) {
    Serial.println("KEY_S_8");
    Keyboard.press(KEYPAD_SUBTRACT);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_8)) {}
    Keyboard.release(KEYPAD_SUBTRACT);
  }
  if (!digitalRead(KEY_S_4)) {
    Serial.println("KEY_S_4");
    Keyboard.press(KEYPAD_ADD);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_4)) {}
    Keyboard.release(KEYPAD_ADD);
  }
  if (!digitalRead(KEY_S_3)) {
    Serial.println("KEY_S_3");
    Keyboard.press(KEY_F);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_3)) {}
    Keyboard.release(KEY_F);
  }
  if (!digitalRead(KEY_S_2)) {
    Serial.println("KEY_S_2");
    Keyboard.press(KEY_UP);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_2)) {}
    Keyboard.release(KEY_UP);
  }
  if (!digitalRead(KEY_S_1)) {
    Serial.println("KEY_S_1");
    Keyboard.press(KEY_F5);
    delay(DEBOUNCE);
    while (!digitalRead(KEY_S_1)) {}
    Keyboard.release(KEY_F5);
  }


  ws2812b.setPixelColor(ledCounter, ws2812b.Color(colors, (colors > 170? colors - 170: 0), 0));
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (ledCounter > 8) {
      if (colors >= 255) {
        updownCount = false;
      }
      if (colors <= 85) {
        updownCount = true;
      }

      if (updownCount)
        colors++;
      else
        colors--;

      ledCounter = 0;
    } else {
      ledCounter++;
    }
  }

  if (false) {
    ws2812b.setPixelColor(ledCounter, Wheel(((ledCounter * 256 / ws2812b.numPixels()) + colors) & 255));

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      if (ledCounter > 8) {
        if (colors > 256 * 5) {
          colors = 0;
        } else {
          colors++;
        }

        ledCounter = 0;
      } else {
        ledCounter++;
      }
    }
  }
  ws2812b.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return ws2812b.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return ws2812b.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return ws2812b.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
