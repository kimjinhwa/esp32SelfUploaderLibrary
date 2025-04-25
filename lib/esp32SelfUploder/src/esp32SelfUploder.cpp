#include <Arduino.h>
#include "esp32SelfUploder.h"

#include <WiFi.h>
//#include <NetworkClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPUpdateServer.h>
#include <Update.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h>

#ifndef STASSID
#define STASSID "iptime_mbhong" 
#define STAPSK  ""
#endif

const char *host = "esp32-webupdate";
const char *ssid = STASSID;
const char *password = STAPSK;

WebServer httpServer(80);
HTTPUpdateServer httpUpdater;

// 객체 정의 추가

// 정적 함수 추가
static void taskLoop(void *pvParameters) {
    ESP32SelfUploder* self = (ESP32SelfUploder*)pvParameters;
    while(1) {
        self->loop();
        delay(1);
    }
}

void printProgress(size_t prg, size_t sz) {
    static int lastProgress = -1;
    int progress = (prg * 100) / sz;
    if (progress != lastProgress) {
        lastProgress = progress;
        Serial.printf("Progress: %d%%\n", progress);
        Serial.printf("Bytes: %d/%d\n", prg, sz);
    }
}

// 버전 체크 및 자동 업데이트 함수 추가
void ESP32SelfUploder::checkAndUpdate(const char* firmware_url) {
    WiFiClient client;
    
    Serial.println("Checking for firmware updates...");
    
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, firmware_url);
    
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", 
                ESPhttpUpdate.getLastError(),
                ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("No updates available");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("Update successful!");
            // 업데이트 성공 후 자동 재시작
            ESP.restart();
            break;
    }
}

void ESP32SelfUploder::begin() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Booting Sketch...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        Serial.println("WiFi failed, retrying.");
    }
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.subnetMask());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.dnsIP());

    if (MDNS.begin(host)) {
        Serial.println("mDNS responder started");
    }

    // Update 진행 상황 콜백 설정
    Update.onProgress(printProgress);

    httpUpdater.setup(&httpServer);
    httpServer.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    
    xTaskCreate(
        taskLoop,
        "ESP32SelfUploder",
        4096,
        this,
        5,
        NULL);

    // 예시: 시작할 때 자동으로 업데이트 체크
    // checkAndUpdate("http://your-server.com/firmware.bin");
}

void ESP32SelfUploder::loop() {
    httpServer.handleClient();
    delay(1);
}