#ifndef ESP32SELFUPLODER_H
#define ESP32SELFUPLODER_H

#include <ArduinoJson.h>

struct FirmwareInfo {
    char latest[100];
    char filename[100];
};

class ESP32SelfUploder {
    private:
        FirmwareInfo currentFirmware;
        static const int FIRMWARE_INFO_ADDR = 0;
        
        bool checkNewVersion(const char* version_url);
        
    public:
        char ssid[32];
        char password[32];
        unsigned int ledPin=15;
        void setLed(unsigned int pin){
            ledPin = pin;
            pinMode(ledPin , OUTPUT);
        };
        ESP32SelfUploder() {
        }

        void begin(const char* ssid, const char* password, const char* update_url);
        void loop();
        bool tryAutoUpdate(const char* firmware_url);
};

extern ESP32SelfUploder selfUploder;
#endif
