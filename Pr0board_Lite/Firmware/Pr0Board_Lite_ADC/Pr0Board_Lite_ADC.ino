#include "DigiKeyboard.h"

void setup() {
  // don't need to set anything up to use DigiKeyboard
}


void loop() {

  
  // Type out this string letter by letter on the computer (assumes US-style
  // keyboard)
  DigiKeyboard.println(readADC(),DEC);

}

int readADC() {
  int avrCount = 10;
  int avrADC = 0;
  for (int i = 0; i < avrCount; ++i)
  {
    avrADC += analogRead(1);
  }
  avrADC /= avrCount;
  return avrADC;
}
