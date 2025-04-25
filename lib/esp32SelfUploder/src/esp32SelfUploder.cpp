#include <Arduino.h>
#include "esp32SelfUploder.h"
#include "../../../version.h"

#include <WiFi.h>
//#include <NetworkClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPUpdateServer.h>
#include <Update.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
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

String updateFile_url;
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
        Serial.printf("Progress: %d%%  ", progress);
        Serial.printf("Bytes: %d/%d\n", prg, sz);
    }
}

// 버전 체크 및 자동 업데이트 함수 추가
bool ESP32SelfUploder::tryAutoUpdate(const char* firmware_url) {
    WiFiClientSecure client;
    HTTPClient http;
    
    Serial.printf("Checking for firmware at: %s\n", firmware_url);
    
    // SSL 인증서 검증 건너뛰기
    client.setInsecure();
    
    // 타임아웃 설정 추가
    client.setTimeout(30000);  // 12초
    http.setTimeout(30000);    // 12초
    
    // 리다이렉션 허용
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    
    http.begin(client, firmware_url);
    
    // 일반 HTTP 요청을 위한 헤더
    http.addHeader("User-Agent", "ESP32");
    
    Serial.println("Starting HTTP GET request...");
    int httpCode = http.GET();
    Serial.printf("HTTP Response code: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
        Serial.println("New firmware found! Starting update...");
        int contentLength = http.getSize();
        Serial.printf("contentLength: %d\n", contentLength);

        // 진행률 표시 설정
        Update.onProgress(printProgress);
   
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



bool ESP32SelfUploder::checkNewVersion(const char* update_url) {
    WiFiClientSecure client;
    HTTPClient http;
    
    client.setInsecure();
    String version_url = String(update_url) + "/version.json";

    Serial.printf("version_url: %s\n", version_url.c_str());
    http.begin(client, version_url);
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        Serial.println("Received JSON:");
        Serial.println(payload);
 
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            const char* latest_version = doc["latest"];
            const char* filename = doc["filename"];
            
            Serial.printf("latest_version pointer: %p\n", latest_version);
            Serial.printf("filename pointer: %p\n", filename);
 
            if (!latest_version || !filename) {
                Serial.println("버전 정보나 파일명이 없습니다");
                http.end();
                return false;
            }
            
            Serial.printf("현재 버전: %s\n", VERSION);
            Serial.printf("서버 버전: %s\n", latest_version);
            Serial.printf("업데이트 파일: %s\n", filename);
            
            // 문자열 비교
            if (String(VERSION) != String(latest_version)) {
                Serial.println("새로운 버전이 있습니다");
                // updateFile_url 생성
                // GitHub rate limiting을 고려한 지연 추가
                Serial.println("GitHub rate limiting을 고려하여 5초 대기...");
                delay(1000);
                updateFile_url = String(update_url) + "/" + String(filename);
                return true;
            } else {
                Serial.println("이미 최신 버전입니다");
            }
        } else {
            Serial.print("JSON 파싱 실패: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.printf("HTTP GET 실패, 에러: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return false;
}
void ESP32SelfUploder::begin(const char* update_url) {
    // EEPROM에서 현재 버전 정보 로드
    
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

    // 버전 정보 URL 구성 (update_url에서 .bin을 .json으로 변경)
    String version_url = String(update_url);
    
    // 새 버전 확인
    if (checkNewVersion(version_url.c_str())) {
        Serial.println("New version available!");
        if (tryAutoUpdate(updateFile_url.c_str())) {
            return;
        }
    } else {
        Serial.println("Already on latest version");
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