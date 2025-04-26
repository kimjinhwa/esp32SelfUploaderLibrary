#include <Arduino.h>
#include "esp32SelfUploder.h"

ESP32SelfUploder selfUploder;
void setup() {
  Serial.begin(115200);
  
  // PSRAM 초기화 및 확인
  
  selfUploder.begin("AndroidHotspot1953", "87654321", "https://raw.githubusercontent.com/kimjinhwa/IP-Fineder-For-ESP32/main/dist/poscot52");
  selfUploder.setLed(15);
}

void loop() {
  Serial.println("Hello, World!");
    Serial.println("PSRAM 초기화... ");
  if(psramInit()) {
    Serial.println("PSRAM 초기화 성공");
    Serial.printf("PSRAM 용량: %d bytes\n", ESP.getPsramSize());
  } else {
    Serial.println("PSRAM 초기화 실패");
  }
  delay(1000);
}
