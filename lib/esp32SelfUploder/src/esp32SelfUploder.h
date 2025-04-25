#ifndef ESP32SELFUPLODER_H
#define ESP32SELFUPLODER_H

#include <ArduinoJson.h>

struct FirmwareInfo {
    char latest[10];
    char filename[10];
};

class ESP32SelfUploder {
    private:
        FirmwareInfo currentFirmware;
        static const int FIRMWARE_INFO_ADDR = 0;
        
        bool checkNewVersion(const char* version_url);
        
    public:
        ESP32SelfUploder() {
        }
        void begin(const char* update_url = nullptr);
        void loop();
        bool tryAutoUpdate(const char* firmware_url);
};

extern ESP32SelfUploder selfUploder;
#endif
