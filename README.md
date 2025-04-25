##  라이브러리 사용방법 
    - 업로드 하는 파일의 파티션 정보는 일치하여야 한다.
    - board_build.partitions = partitions.csv
    - 자동 업그레이드 Wifi 연결은 
    SSID : AndroidHotspot1953
    PASS : 87654321
    - update시 LED가 빠르게 깜빡인다.
    ```
    # name,   type, subtype, offset,  size, flags
    nvs,      data, nvs,     0x9000,  0x5000,
    otadata,  data, ota,     0xe000,  0x2000,
    app0,     app,  ota_0,   0x10000, 0x200000,
    app1,     app,  ota_1,   0x300000,0x200000,
    spiffs,   data, spiffs,  0x500000,0x2F0000,
    coredump, data, coredump,0x7F0000,0x10000,
    ```

    - 프로젝트 루트에 version.h파일을 만들고
    그 안에 
    #define 1.0.0 // ....주석...
    과 같이 만든다. 
    - platform.io에 
    ```
    build_flags = 
	-DAUTOUPDATE=1
    ```
    - selfUploder.begin("https://raw.githubusercontent.com/kimjinhwa/IP-Fineder-For-ESP32/main/dist/poscot52");
    - 이제 C:\DevWork\pythonWork\udpBroadCast\dist\poscot52 에가서 version.json 과 새로운 펌웨어를 넣고. 
    - .\gitversiontag를 실행한다. 
    새로운 버전을 모든 단말은 와이파이에 연결되어 있다면 업데이트를 진행 할 것이다. 

    - version.json 
    ```
    {
    "latest": "1.0.2",
    "filename": "firmware_T52.bin"
    }
    ```