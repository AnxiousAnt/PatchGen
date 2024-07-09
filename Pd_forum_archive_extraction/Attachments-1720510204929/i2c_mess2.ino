#include <Wire.h>

void setup(void) {
  pinMode(13, OUTPUT);
  Wire.begin(0x09);
  Wire.onReceive(receiveEvent);
}

void loop() {

}

void receiveEvent(int howMany) {
  byte inByte;
  while (Wire.available()) {
    inByte = Wire.read();
  }
  digitalWrite(13, inByte);
}
