#ifndef ESP32SELFUPLODER_H
#define ESP32SELFUPLODER_H

class ESP32SelfUploder {
    public:
        ESP32SelfUploder(){};
        void begin();
        void loop();
        void checkAndUpdate(const char* firmware_url);
};
extern ESP32SelfUploder selfUploder;
#endif
