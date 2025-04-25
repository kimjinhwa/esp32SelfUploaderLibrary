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
        Serial.printf("Progress: %d%%  ", progress);
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
    //http.addHeader("Accept", "*/*");
    //http.addHeader("Cache-Control", "no-cache");
    http.addHeader("Accept", "application/octet-stream");
    
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

bool ESP32SelfUploder::loadFirmwareInfo() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(FIRMWARE_INFO_ADDR, currentFirmware);
    EEPROM.end();
    return true;
}

bool ESP32SelfUploder::saveFirmwareInfo() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(FIRMWARE_INFO_ADDR, currentFirmware);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

Version ESP32SelfUploder::parseVersion(const char* version_str) {
    Version ver = {0, 0, 0};
    sscanf(version_str, "%hhu.%hhu.%hhu", &ver.major, &ver.minor, &ver.patch);
    return ver;
}

bool ESP32SelfUploder::checkNewVersion(const char* version_url) {
    WiFiClientSecure client;
    HTTPClient http;
    
    client.setInsecure();
    http.begin(client, version_url);
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            const char* latest_version = doc["latest"];
            Version serverVersion = parseVersion(latest_version);
            
            // 현재 버전과 비교
            if (serverVersion > currentFirmware.version) {
                // 새 버전이 있음
                for (JsonVariant v : doc["files"].as<JsonArray>()) {
                    if (strcmp(v["version"], latest_version) == 0) {
                        const char* url = v["url"];
                        const char* md5 = v["md5"];
                        // 업데이트 정보 저장
                        currentFirmware.version = serverVersion;
                        strcpy(currentFirmware.md5, md5);
                        currentFirmware.timestamp = v["timestamp"];
                        return true;
                    }
                }
            }
        }
    }
    
    http.end();
    return false;
}

void ESP32SelfUploder::begin(const char* update_url) {
    // EEPROM에서 현재 버전 정보 로드
    loadFirmwareInfo();
    
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
    version_url.replace(".bin", ".json");
    
    // 새 버전 확인
    if (checkNewVersion(version_url.c_str())) {
        Serial.println("New version available!");
        if (tryAutoUpdate(update_url)) {
            saveFirmwareInfo();  // 성공적으로 업데이트되면 정보 저장
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