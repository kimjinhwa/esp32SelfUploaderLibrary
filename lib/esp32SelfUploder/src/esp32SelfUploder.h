#ifndef ESP32SELFUPLODER_H
#define ESP32SELFUPLODER_H

class ESP32SelfUploder {
    public:
        ESP32SelfUploder(){};
        void begin(const char* update_url = nullptr);
        void loop();
        bool tryAutoUpdate(const char* firmware_url);
};
extern ESP32SelfUploder selfUploder;
#endif
