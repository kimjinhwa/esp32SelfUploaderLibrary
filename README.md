# 사용방법 
    - 프로젝트 루트에 version.h파일을 만들고
    그 안에 
    #define 1.0.0 // ....주석...
    과 같이 만든다. 
    - selfUploder.begin("https://raw.githubusercontent.com/kimjinhwa/IP-Fineder-For-ESP32/main/dist/poscot52");
    - 이제 C:\DevWork\pythonWork\udpBroadCast\dist\poscot52 에가서 version.json 과 새로운 펌웨어를 넣고. 
    - .\gitversiontag를 실행한다. 

    새로운 버전을 모든 단말은 와이파이에 연결되어 있다면 업데이트를 진행 할 것이다. 