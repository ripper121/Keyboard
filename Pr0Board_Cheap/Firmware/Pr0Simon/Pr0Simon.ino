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

  ws2812b.begin();
  ws2812b.setBrightness(50);
  ws2812b.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0));

}

byte ledCounter = 0;
int colors = 0;
unsigned long previousMillis = 0;
const long interval = 1;
bool updownCount = false;
byte simonSays[1024];
int simonCount = 0;

void loop()
{
  simonSays[simonCount++] = random(4);

  //show sequence
  for (int i = 0; i < simonCount; i++) {
    ws2812b.clear();
    ws2812b.show();
    delay(600);
    if (simonSays[i] == 0) {
      ws2812b.setPixelColor(0, ws2812b.Color(255, 0, 0));
    }
    if (simonSays[i] == 1) {
      ws2812b.setPixelColor(1, ws2812b.Color(0, 0, 255));
    }
    if (simonSays[i] == 2) {
      ws2812b.setPixelColor(5, ws2812b.Color(255, 255, 0));
    }
    if (simonSays[i] == 3) {
      ws2812b.setPixelColor(4, ws2812b.Color(0, 255, 0));
    }
    ws2812b.show();
    delay(600);
  }
  ws2812b.setPixelColor(0, ws2812b.Color(255, 0, 0));
  ws2812b.setPixelColor(1, ws2812b.Color(0, 0, 255));
  ws2812b.setPixelColor(5, ws2812b.Color(255, 255, 0));
  ws2812b.setPixelColor(4, ws2812b.Color(0, 255, 0));
  ws2812b.show();

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
  if (rightSequence) {
    ws2812b.clear();
    ws2812b.setPixelColor(2, ws2812b.Color(0, 255, 0));
    ws2812b.setPixelColor(3, ws2812b.Color(0, 255, 0));
    ws2812b.setPixelColor(6, ws2812b.Color(0, 255, 0));
    ws2812b.setPixelColor(7, ws2812b.Color(0, 255, 0));
    ws2812b.show();
  } else {
    ws2812b.clear();
    ws2812b.setPixelColor(2, ws2812b.Color(255, 0, 0));
    ws2812b.setPixelColor(3, ws2812b.Color(255, 0, 0));
    ws2812b.setPixelColor(6, ws2812b.Color(255, 0, 0));
    ws2812b.setPixelColor(7, ws2812b.Color(255, 0, 0));
    ws2812b.show();
    simonCount = 0;
    randomSeed(millis());
  }
  delay(1000);
}

byte getButton() {
  while (true) {
    if (!digitalRead(KEY_S_1)) {
      ws2812b.setPixelColor(0, ws2812b.Color(0, 0, 0));
      ws2812b.show();
      while (!digitalRead(KEY_S_1)) {delay(20);}
      ws2812b.setPixelColor(0, ws2812b.Color(255, 0, 0));
      ws2812b.show();
      return 0;
    }
    if (!digitalRead(KEY_S_2)) {
      ws2812b.setPixelColor(1, ws2812b.Color(0, 0, 0));
      ws2812b.show();
      while (!digitalRead(KEY_S_2)) {delay(20);}
      ws2812b.setPixelColor(1, ws2812b.Color(0, 0, 255));
      ws2812b.show();
      return 1;
    }
    if (!digitalRead(KEY_S_5)) {
      ws2812b.setPixelColor(4, ws2812b.Color(0, 0, 0));
      ws2812b.show();
      while (!digitalRead(KEY_S_5)) {delay(20);}
      ws2812b.setPixelColor(4, ws2812b.Color(0, 255, 0));
      ws2812b.show();
      return 3;
    }
    if (!digitalRead(KEY_S_6)) {
      ws2812b.setPixelColor(5, ws2812b.Color(0, 0, 0));
      ws2812b.show();
      while (!digitalRead(KEY_S_6)) {delay(20);}
      ws2812b.setPixelColor(5, ws2812b.Color(255, 255, 0));
      ws2812b.show();
      return 2;
    }
  }
}
