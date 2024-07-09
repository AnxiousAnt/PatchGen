#include <Wire.h>

void setup(void) {
  Wire.begin(0x0A);
  Wire.onReceive(receiveEvent);
  
  Serial.begin(115200);
  while (!Serial);

  Serial.println("I2C address bug test");
}

void loop() {

}

void receiveEvent(int howMany) {
  int index = 0;
  while (Wire.available()) {
    Serial.print(Wire.read());
    Serial.print(" ");
  }
  Serial.println();
}
