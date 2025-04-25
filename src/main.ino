#include <Arduino.h>
#include "esp32SelfUploder.h"

ESP32SelfUploder selfUploder;
void setup() {
  Serial.begin(115200);
  selfUploder.begin("AndroidHotspot1953", "87654321", "https://raw.githubusercontent.com/kimjinhwa/IP-Fineder-For-ESP32/main/dist/poscot52");
  selfUploder.setLed(15);
}

void loop() {
  Serial.println("Hello, World!");
  delay(1000);
}
