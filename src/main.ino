#include <Arduino.h>
#include "esp32SelfUploder.h"

ESP32SelfUploder selfUploder;
void setup() {
  Serial.begin(115200);
  selfUploder.begin("https://github.com/kimjinhwa/IP-Fineder-For-ESP32/raw/refs/heads/main/dist/firmware_T52.bin");
}

void loop() {
  Serial.println("Hello, World!");
  delay(1000);
}
