#include <Arduino.h>
#include "esp32SelfUploder.h"

ESP32SelfUploder selfUploder;
void setup() {
  Serial.begin(115200);
  selfUploder.begin();
}

void loop() {
  Serial.println("Hello, World!");
  delay(1000);
}
