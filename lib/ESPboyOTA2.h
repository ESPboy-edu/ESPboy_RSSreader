/*
ESPboyOTA2 class AppStore (OTA2) v4.0
www.ESPboy.com
Proprietary license 2021 (C) RomanS espboy.edu@gmail.com
*/

#ifndef ESPboy_OTA
#define ESPboy_OTA

#define OTAv 4.0

#include <FS.h> 
using fs::FS;
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESPboyInit.h"
#include "ESPboyTerminalGUI.h"
#include "ESPboyLED.h"

#define OTA_TIMEOUT_CONNECTION 10000


struct wificlient{
  String ssid;
  String pass;
};


struct wf {
    String ssid;
    uint8_t rssi;
    uint8_t encription;
};


struct lessRssi{
    inline bool operator() (const wf& str1, const wf& str2) {return (str1.rssi > str2.rssi);}
};


struct iL {
  uint32_t iData;
  int8_t iAccsess;
  int16_t iLike;
  String iItem;
};


struct globalStruct{
  uint8_t accessUser;
  uint32_t userLineNo;
  uint32_t appLineNo;
  uint32_t lastVisitTimestamp;
  String nicknameUser;
  String type;
  String genre;
  String app;
  String updlink;
};


struct messStruct{
 String   messmess;
 uint16_t messcolor;
 uint32_t messdelay;
}; 


    enum GET_CMD {GET_TYPE=1, GET_GENRE, GET_APP, GET_INFO, GET_MESS, GET_USER, GET_UPDATE, GET_LIKE};
    enum POST_CMD {POST_CONNECT=1, POST_LIKE, POST_DOWNLOAD, POST_LOG_LIKE};
    enum RESULT_STATE {FAULT_STATE=0, OK_STATE, BACK_STATE, LIKE_STATE};
    enum MESS {START_MESS=0, USERFOUND_MESS, USERNOTFOUND_MESS, BEFOREUPDATE_MESS, UPDATEFINISHWD_MESS};

class ESPboyOTA2{
  private:
    ESPboyTerminalGUI *terminalGUIobj = NULL;
    
    std::vector<wf> wfList; 
    std::vector<iL> iList;
    
    const static char* PROGMEM googlURLstart;
    const static char* PROGMEM googlURLend;
    const static char* PROGMEM googlLOGurl;
    const static char* PROGMEM googlUSERurl;
    const static char* PROGMEM googlMESSurl;
    const static char* PROGMEM googlLISTurl;
    const static char* PROGMEM googlLIKEurl;
    const static char* PROGMEM googlDAWNLOADurl;

    const static char* PROGMEM finalMnu[];
    const static char* PROGMEM itemBck;
  
    void checkOTA();
    void OTAprogress(int cur, int total);
    String GETmethod(GET_CMD GETcmd);
    String POSTmethod(POST_CMD POST_cmd);
    String fillPayload(String linkField);
    String getWiFiStatusName();
    uint16_t scanWiFi();
    bool wifiConnect();
    bool UPDATEmethod();
    RESULT_STATE get_mess();
    RESULT_STATE get_user();
    RESULT_STATE get_type();
    RESULT_STATE get_genre();
    RESULT_STATE get_app();
    RESULT_STATE get_info();
    
  
public:
   ESPboyOTA2(ESPboyTerminalGUI* GUIobjOTA);
};

#endif
