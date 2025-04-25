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
#include <WiFiClientSecure.h>
//#include <ESP32httpUpdate.h>

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
bool ESP32SelfUploder::tryAutoUpdate(const char* firmware_url) {
    WiFiClientSecure client;
    HTTPClient http;
    
    Serial.printf("Checking for firmware at: %s\n", firmware_url);
    
    // GitHub의 인증서 검증 건너뛰기
    client.setInsecure();
    
    // 타임아웃 설정 추가
    client.setTimeout(12000);  // 12초
    http.setTimeout(12000);    // 12초
    
    // 리다이렉션 허용
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    
    http.begin(client, firmware_url);
    
    // GitHub API 요청을 위한 헤더 추가
    http.addHeader("User-Agent", "ESP32");
    http.addHeader("Accept", "*/*");
    http.addHeader("Cache-Control", "no-cache");
    
    Serial.println("Starting HTTP GET request...");
    int httpCode = http.GET();
    Serial.printf("HTTP Response code: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
        Serial.println("New firmware found! Starting update...");
        int contentLength = http.getSize();
        
        if (Update.begin(contentLength)) {
            Serial.printf("Starting OTA: %d bytes\n", contentLength);
            size_t written = Update.writeStream(http.getStream());
            
            if (written == contentLength) {
                Serial.println("Written : " + String(written) + " successfully");
                if (Update.end()) {
                    Serial.println("OTA update complete!");
                    ESP.restart();
                    return true;
                } else {
                    Serial.println("Error Occurred: " + String(Update.getError()));
                }
            } else {
                Serial.println("Write Error Occurred. Written only : " + String(written) + "/" + String(contentLength));
            }
        } else {
            Serial.println("Not enough space to begin OTA");
        }
    } else {
        Serial.printf("Firmware not found, HTTP error: %d\n", httpCode);
        Serial.println(http.errorToString(httpCode));
        if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
            Serial.println("Connection refused");
        } else if (httpCode == HTTPC_ERROR_SEND_HEADER_FAILED) {
            Serial.println("Send header failed");
        } else if (httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED) {
            Serial.println("Send payload failed");
        } else if (httpCode == HTTPC_ERROR_NOT_CONNECTED) {
            Serial.println("Not connected");
        } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
            Serial.println("Connection lost");
        } else if (httpCode == HTTPC_ERROR_NO_HTTP_SERVER) {
            Serial.println("No HTTP server");
        }
    }
    
    http.end();
    return false;
}

void ESP32SelfUploder::begin(const char* update_url) {
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

    // 자동 업데이트 시도
    if (update_url != nullptr) {
        if (!tryAutoUpdate(update_url)) {
            Serial.println("Auto update not available, starting web interface...");
        } else {
            return; // 업데이트 성공시 여기서 종료 (재시작됨)
        }
    }

    // 웹 인터페이스 시작
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
}

void ESP32SelfUploder::loop() {
    httpServer.handleClient();
    delay(1);
}