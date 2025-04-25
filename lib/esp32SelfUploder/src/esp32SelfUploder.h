#ifndef ESP32SELFUPLODER_H
#define ESP32SELFUPLODER_H

#include <EEPROM.h>
#include <ArduinoJson.h>

struct Version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    
    bool operator>(const Version& other) const {
        if (major != other.major) return major > other.major;
        if (minor != other.minor) return minor > other.minor;
        return patch > other.patch;
    }
    
    String toString() const {
        return String(major) + "." + String(minor) + "." + String(patch);
    }
};

struct FirmwareInfo {
    Version version;
    char md5[33];
    uint32_t timestamp;
};

class ESP32SelfUploder {
    private:
        FirmwareInfo currentFirmware;
        static const int EEPROM_SIZE = 512;
        static const int FIRMWARE_INFO_ADDR = 0;
        
        bool loadFirmwareInfo();
        bool saveFirmwareInfo();
        bool checkNewVersion(const char* version_url);
        Version parseVersion(const char* version_str);
        
    public:
        ESP32SelfUploder() {
            currentFirmware.version = {1, 0, 0};  // 초기 버전
            strcpy(currentFirmware.md5, "");
            currentFirmware.timestamp = 0;
        }
        void begin(const char* update_url = nullptr);
        void loop();
        bool tryAutoUpdate(const char* firmware_url);
        Version getCurrentVersion() { return currentFirmware.version; }
};

extern ESP32SelfUploder selfUploder;
#endif
