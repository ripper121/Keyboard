#include <WS2812.h>


#define PIN            0
#define NUMPIXELS      8
cRGB value;
const float brightness = 1;
WS2812 LED(NUMPIXELS); // 1 LED

void ledSet(byte r, byte g, byte b, float brightness) {
  value.r = r * brightness; value.g = g * brightness; value.b = b * brightness;
  for (byte i = 0; i < NUMPIXELS; i++) {
    LED.set_crgb_at(i, value); // Set value at LED found at index 0
    delay(100);
    LED.sync(); // Sends the value to the LED
  }
}

void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  analogReference(DEFAULT);

  LED.setOutput(PIN); // Digital Pin 9
  ledSet(255, 64, 0, brightness);

}

void loop() {
  ledSet(255, 0, 0, brightness);
  ledSet(0, 255, 0, brightness);
  ledSet(0, 0, 255, brightness);
  ledSet(255, 255, 255, brightness);
}
