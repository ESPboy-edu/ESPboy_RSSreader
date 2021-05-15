//ESPboy RSS reader v1.0 adopted 15.05.2021 by RomanS for www.espboy.com project

/*
 *  RSSReader.h - Header file for RSSReader Class
 *
 *  Concept, Design and Implementation by: Craig A. Lindley
 *  Last Update: 11/06/2015
 */


#include <ESP8266WiFi.h>
#include "RSSReader.h"

#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include "lib/ESPboyTerminalGUI.h"
#include "lib/ESPboyTerminalGUI.cpp"

ESPboyInit myESPboy;
ESPboyTerminalGUI terminalGUIobj(&myESPboy.tft, &myESPboy.mcp);


// Array of feed URLs
const char *rssFeedURLs [][2] = {
 {"http://feeds.bbci.co.uk/news/world/rss.xml", "BBC world"},
 {"http://rss.cnn.com/rss/edition_europe.rss", "CNN Europe"},
 {"http://rss.cnn.com/rss/cnn_topstories.rss", "CNN top stories"},
 {"rssfeeds.usatoday.com/usatoday-NewsTopStories", "USA today top"},
};

#define NUMBER_OF_FEEDS (sizeof(rssFeedURLs) / (sizeof(char *)*2))
#define WIFI_TIMEOUT_CONNECTION 10000

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


RSSReader reader = RSSReader(100);
wificlient *wificl;
std::vector<wf> wfList; 


bool smartDelay(uint32_t timeDelay){
  uint32_t timerCount = millis();
  while (millis()-timerCount < timeDelay && !myESPboy.getKeys()){delay (200);}
  return (!(!(myESPboy.getKeys()&PAD_ESC)));
};

bool titleCallback(const char *titleStr) {
  terminalGUIobj.printConsole(titleStr, TFT_WHITE, 1, 0); 
  return(smartDelay(5000));
}

bool  pubDateCallback(const char *dateStr) {
  bool stopFlag=false;
  terminalGUIobj.printConsole(dateStr, TFT_YELLOW, 1, 0);
  return(smartDelay(100));
}

bool descCallback(const char *descStr) {
  bool stopFlag=false;
  terminalGUIobj.printConsole(descStr, TFT_MAGENTA, 1, 0);
  return(smartDelay(15000));
}



String getWiFiStatusName() {
  String stat;
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      stat = (F("Idle"));
      break;
    case WL_NO_SSID_AVAIL:
      stat = (F("No SSID available"));
      break;
    case WL_SCAN_COMPLETED:
      stat = (F("Scan completed"));
      break;
    case WL_CONNECTED:
      stat = (F("WiFi connected"));
      break;
    case WL_CONNECT_FAILED:
      stat = (F("Wrong passphrase"));
      break;
    case WL_CONNECTION_LOST:
      stat = (F("Connection lost"));
      break;
    case WL_DISCONNECTED:
      stat = (F("Wrong password"));
      break;
    default:
      stat = (F("Unknown"));
      break;
  };
  return stat;
}



uint16_t scanWiFi() {
  terminalGUIobj.printConsole(F("Scaning WiFi..."), TFT_MAGENTA, 1, 0);
  int16_t WifiQuantity = WiFi.scanNetworks();
  if (WifiQuantity != -1 && WifiQuantity != -2 && WifiQuantity != 0) {
    for (uint8_t i = 0; i < WifiQuantity; i++) wfList.push_back(wf());
    if (!WifiQuantity) {
      terminalGUIobj.printConsole(F("WiFi not found"), TFT_RED, 1, 0);
      delay(3000);
      ESP.reset();
    } else
      for (uint8_t i = 0; i < wfList.size(); i++) {
        wfList[i].ssid = WiFi.SSID(i);
        wfList[i].rssi = WiFi.RSSI(i);
        wfList[i].encription = WiFi.encryptionType(i);
        delay(0);
      }
    sort(wfList.begin(), wfList.end(), lessRssi());
    return (WifiQuantity);
  } else
    return (0);
}



bool wifiConnect() {
 uint16_t wifiNo = 0;
 uint32_t timeOutTimer;
 static uint8_t connectionErrorFlag = 0;

  wificl = new wificlient();
  
  if (!connectionErrorFlag && !(terminalGUIobj.getKeys()&PAD_ESC)) {
    wificl->ssid = WiFi.SSID();
    wificl->pass = WiFi.psk();
    terminalGUIobj.printConsole(F("Last network:"), TFT_MAGENTA, 0, 0);
    terminalGUIobj.printConsole(wificl->ssid, TFT_GREEN, 0, 0);
  } 
  else 
  {
      wificl->ssid = "";
      wificl->pass = "";
    
    if (scanWiFi())
      for (uint8_t i = wfList.size(); i > 0; i--) {
        String toPrint =
            (String)(i) + " " + wfList[i - 1].ssid + " " + wfList[i - 1].rssi +
            "" + ((wfList[i - 1].encription == ENC_TYPE_NONE) ? "" : "*");
        terminalGUIobj.printConsole(toPrint, TFT_YELLOW, 0, 0);
    }

    while (!wifiNo) {
      terminalGUIobj.printConsole(F("Choose WiFi No:"), TFT_MAGENTA, 0, 0);
      wifiNo = terminalGUIobj.getUserInput().toInt();
      if (wifiNo < 1 || wifiNo > wfList.size()) wifiNo = 0;
    }

    wificl->ssid = wfList[wifiNo - 1].ssid;
    terminalGUIobj.printConsole(wificl->ssid, TFT_GREEN, 1, 0);

    while (!wificl->pass.length()) {
      terminalGUIobj.printConsole(F("Password:"), TFT_MAGENTA, 0, 0);
      wificl->pass = terminalGUIobj.getUserInput();
    }
    terminalGUIobj.printConsole(/*pass*/F("******"), TFT_GREEN, 0, 0);
  }

  wfList.clear();

  WiFi.mode(WIFI_STA);
  WiFi.begin(wificl->ssid, wificl->pass);

  terminalGUIobj.printConsole(F("Connection..."), TFT_MAGENTA, 0, 0);
  timeOutTimer = millis();
  String dots = "";
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - timeOutTimer < WIFI_TIMEOUT_CONNECTION)) {
    delay(700);
    terminalGUIobj.printConsole(dots, TFT_MAGENTA, 0, 1);
    dots += ".";
  }

  if (WiFi.status() != WL_CONNECTED) {
    connectionErrorFlag = 1;
    terminalGUIobj.printConsole(getWiFiStatusName(), TFT_RED, 0, 1);
    delay(1000);
    terminalGUIobj.printConsole("", TFT_BLACK, 0, 0);
    delete (wificl);
    return (false);
  } else {
    terminalGUIobj.printConsole(getWiFiStatusName(), TFT_MAGENTA, 0, 1);
    delete (wificl);
    return (true);
  }
}



void setup() {
  Serial.begin(115200);
  myESPboy.begin("RSS reader");
  WiFi.mode(WIFI_STA);
  
  terminalGUIobj.toggleDisplayMode(1);
  while (!wifiConnect());
  
  reader.setTitleCallback(&titleCallback);
  reader.setPubDateCallback(&pubDateCallback);
  reader.setDescCallback(&descCallback);
}



void loop() {
  static uint8_t rssFeedIndex=0;
  const char *url;
  String toPrint;

  terminalGUIobj.printConsole(F(""), TFT_BLACK, 0, 0);
  for(uint8_t i=0; i<NUMBER_OF_FEEDS; i++){
    toPrint=i+1;
    toPrint+=" ";
    toPrint+=rssFeedURLs[i][1];
    terminalGUIobj.printConsole(toPrint, TFT_YELLOW, 0, 0);
  };

  terminalGUIobj.printConsole(F(""), TFT_BLACK, 0, 0);
  uint8_t inputVal=0;
  while (!inputVal || inputVal<0 || inputVal>NUMBER_OF_FEEDS+1){
    inputVal=0;
    terminalGUIobj.printConsole(F("Choose RSS No:"), TFT_MAGENTA, 0, 1);
    inputVal = terminalGUIobj.getUserInput().toInt();
  }

  terminalGUIobj.printConsole(rssFeedURLs[inputVal-1][1], TFT_GREEN, 0, 0);
  terminalGUIobj.printConsole(F(""), TFT_BLACK, 0, 0);
  url=rssFeedURLs[inputVal-1][0];
  
  if(!reader.read(url)) terminalGUIobj.printConsole(F("End"), TFT_GREEN, 1, 0);
  else terminalGUIobj.printConsole(F("Break"), TFT_RED, 1, 0);
  
  delay(100);
}
