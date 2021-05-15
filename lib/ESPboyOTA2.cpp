/*
ESPboyOTA2 class AppStore (OTA2) v4.0
www.ESPboy.com
Proprietary license 2021 (C) RomanS espboy.edu@gmail.com
*/

#include "ESPboyOTA2.h"

    const char* PROGMEM ESPboyOTA2::googlURLstart = "https://script.google.com/macros/s/";
    const char* PROGMEM ESPboyOTA2::googlURLend = "/exec";
    const char* PROGMEM ESPboyOTA2::googlLOGurl = "AKfycbyK38VudmWzFLzQCXpCor6dbjUaVdJyLs678DYUQw";
    const char* PROGMEM ESPboyOTA2::googlUSERurl = "AKfycbz5Pzjixa7cyDsA-ML9KTE9YNg8BkMb90Y6OT45J3UUNWqY2uVm";
    const char* PROGMEM ESPboyOTA2::googlMESSurl = "AKfycbzK7UH3YJb9w27dWCNLqrMKzMjDAAzPY1z1lZGspKTrEFaHDpbp";
    const char* PROGMEM ESPboyOTA2::googlLISTurl = "AKfycbwKMaHpDg_a52oSNdlbO9q1XEDlFMRVbFdKrarnp0HyHoCAGak";
    const char* PROGMEM ESPboyOTA2::googlLIKEurl = "AKfycbySuuY9VLKanprDx9N7kBLiQE1kk3TIXh28GC4iP22FLz0WjP4";
    const char* PROGMEM ESPboyOTA2::googlDAWNLOADurl = "https://drive.google.com/uc?export=download&id=";

    const char* PROGMEM ESPboyOTA2::finalMnu[] = {"1 DOWNLOAD", "2 LIKE", "3 BACK"};
    const char* PROGMEM ESPboyOTA2::itemBck = "BACK\0x0";

    enum GET_CMD GET_cmd;
    enum POST_CMD POST_cmd;
    enum RESULT_STATE GET_result;

    messStruct *messStr;
    globalStruct *globalStr;
    wificlient *wificl;
    

ESPboyOTA2::ESPboyOTA2(ESPboyTerminalGUI* GUIobjOTA) {    
   terminalGUIobj = GUIobjOTA;
   terminalGUIobj->toggleDisplayMode(1);
   checkOTA();
}


void ESPboyOTA2::OTAprogress(int cur, int total) {
  terminalGUIobj->printConsole((String)(cur * 100 / total) + "%", TFT_GREEN, 1, 1);
}


bool ESPboyOTA2::UPDATEmethod() {
 uint8_t header[4];
 int size;
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (http.begin(client, (String)googlDAWNLOADurl+globalStr->updlink)) {
      if (http.GET() == HTTP_CODE_OK) {
        size = http.getSize();
        WiFiClient *tcp = http.getStreamPtr();
        WiFiClient::stopAllExcept(tcp);
        if (tcp->peekBytes(header, sizeof(header)) == sizeof(header))
          if (header[0] == 0xE9)
            if (Update.begin(size, U_FLASH, LED_BUILTIN, LOW)) {
              Update.onProgress([this](int a, int b){this->OTAprogress(a,b);});
              if (Update.writeStream(*tcp) == (size_t)size)
                if (Update.end()) 
                  return (true);
            }
      }
    http.end();
    }
  return (false);
}


String ESPboyOTA2::GETmethod(GET_CMD GETcmd){
 String response;
 String urlstr; 
  switch (GETcmd){
    case GET_USER: urlstr = googlUSERurl; break;
    case GET_MESS: urlstr = googlMESSurl; break;
    default: urlstr = googlLISTurl; 
  }
  
  urlstr = (String)googlURLstart + urlstr + (String)googlURLend;
  urlstr += F("?cmd=");
  urlstr += GETcmd;
  urlstr += F("&dat=");

  switch (GETcmd){
    case GET_USER:
      urlstr += (String)ESP.getChipId();
      urlstr += ';';
      urlstr += WiFi.macAddress();
      break;
    case GET_GENRE:
      urlstr += globalStr->type;
      break;
    case GET_APP:
      urlstr += globalStr->genre;
      break;
    case GET_INFO:
      urlstr += globalStr->app;
      break;
 }
  
  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();
  
  if (http.begin(client, urlstr)) {
    http.setTimeout(OTA_TIMEOUT_CONNECTION);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (http.GET() == HTTP_CODE_OK) {
      response = http.getString();
      http.end();
    }
  } 
 
 return (response);
}
  

String ESPboyOTA2::POSTmethod(POST_CMD POST_cmd){
 String postData;
 String response;
 String googleURL;
 String toPrint;

  switch(POST_cmd){
    case POST_CONNECT:
      googleURL = googlLOGurl;
      toPrint = F("CONNECT ");
      toPrint += globalStr->nicknameUser;
      postData = fillPayload(toPrint);
      break;
    case POST_LIKE:
      googleURL = googlLIKEurl;
      postData = globalStr->nicknameUser + ';' + globalStr->app;
      break;
    case POST_LOG_LIKE:
      googleURL = googlLOGurl;
      toPrint = F("LIKE ");
      toPrint += globalStr->app;
      postData = fillPayload(toPrint);
      break;
    case POST_DOWNLOAD:
      googleURL = googlLOGurl;
      toPrint = F("DOWNLOAD ");
      toPrint += globalStr->app;
      postData = fillPayload(toPrint);
      break;
  }

  
  HTTPClient http;    //Declare object of class HTTPClient
  WiFiClientSecure client;
  client.setInsecure();
   if (http.begin(client, (String)googlURLstart+googleURL+(String)googlURLend)) {
    http.setTimeout(OTA_TIMEOUT_CONNECTION);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (http.POST(postData) == HTTP_CODE_OK) {
      response = http.getString();
      http.end();
    }
  }
 return (response);
}



String ESPboyOTA2::fillPayload(String linkField) {
 String payload = "";
  payload += WiFi.macAddress();   // MAC address
  payload += F("; ");  // date/time
  payload +=F("; "); 
  payload += WiFi.localIP().toString();
  payload += F("; "); 
  payload += linkField;  // download name
  payload += F("; "); 
  payload += (String)ESP.getFreeHeap();
  payload += F("; "); 
  payload += (String)ESP.getFreeContStack();
  payload += F("; "); 
  payload += (String)ESP.getChipId();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipId();
  payload += F("; "); 
  payload += (String)ESP.getCoreVersion();
  payload += F("; "); 
  payload += (String)ESP.getSdkVersion();
  payload += F("; "); 
  payload += (String)ESP.getCpuFreqMHz();
  payload += F("; "); 
  payload += (String)ESP.getSketchSize();
  payload += F("; "); 
  payload += (String)ESP.getFreeSketchSpace();
  payload += F("; "); 
  payload += (String)ESP.getSketchMD5();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipSize();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipRealSize();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipSpeed();
  payload += F("; ");
  payload += (String)ESP.getCycleCount();
  payload += F("; "); 
  payload += WiFi.SSID();
  payload += F("; "); 
  payload +=OTAv;
  payload += F("; ");
  payload += globalStr->accessUser;
  payload += F("; ");
  payload += globalStr->userLineNo;
  payload += F("; ");
  payload += globalStr->appLineNo;
  return (payload);
}



uint16_t ESPboyOTA2::scanWiFi() {
  terminalGUIobj->printConsole(F("Scaning WiFi..."), TFT_MAGENTA, 1, 0);
  int16_t WifiQuantity = WiFi.scanNetworks();
  if (WifiQuantity != -1 && WifiQuantity != -2 && WifiQuantity != 0) {
    for (uint8_t i = 0; i < WifiQuantity; i++) wfList.push_back(wf());
    if (!WifiQuantity) {
      terminalGUIobj->printConsole(F("WiFi not found"), TFT_RED, 1, 0);
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


bool ESPboyOTA2::wifiConnect() {
 uint16_t wifiNo = 0;
 uint32_t timeOutTimer;
 static uint8_t connectionErrorFlag = 0;

  wificl = new wificlient();
  
  if (!connectionErrorFlag && !(terminalGUIobj->getKeys()&PAD_ESC)) {
    wificl->ssid = WiFi.SSID();
    wificl->pass = WiFi.psk();
    terminalGUIobj->printConsole(F("Last network:"), TFT_MAGENTA, 0, 0);
    terminalGUIobj->printConsole(wificl->ssid, TFT_GREEN, 0, 0);
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
        terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 0, 0);
    }

    while (!wifiNo) {
      terminalGUIobj->printConsole(F("Choose WiFi No:"), TFT_MAGENTA, 0, 0);
      wifiNo = terminalGUIobj->getUserInput().toInt();
      if (wifiNo < 1 || wifiNo > wfList.size()) wifiNo = 0;
    }

    wificl->ssid = wfList[wifiNo - 1].ssid;
    terminalGUIobj->printConsole(wificl->ssid, TFT_GREEN, 1, 0);

    while (!wificl->pass.length()) {
      terminalGUIobj->printConsole(F("Password:"), TFT_MAGENTA, 0, 0);
      wificl->pass = terminalGUIobj->getUserInput();
    }
    terminalGUIobj->printConsole(/*pass*/F("******"), TFT_GREEN, 0, 0);
  }

  wfList.clear();

  WiFi.mode(WIFI_STA);
  WiFi.begin(wificl->ssid, wificl->pass);

  terminalGUIobj->printConsole(F("Connection..."), TFT_MAGENTA, 0, 0);
  timeOutTimer = millis();
  String dots = "";
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - timeOutTimer < OTA_TIMEOUT_CONNECTION)) {
    delay(700);
    terminalGUIobj->printConsole(dots, TFT_MAGENTA, 0, 1);
    dots += ".";
  }

  if (WiFi.status() != WL_CONNECTED) {
    connectionErrorFlag = 1;
    terminalGUIobj->printConsole(getWiFiStatusName(), TFT_RED, 0, 1);
    delay(1000);
    terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
    delete (wificl);
    return (false);
  } else {
    terminalGUIobj->printConsole(getWiFiStatusName(), TFT_MAGENTA, 0, 1);
    delete (wificl);
    return (true);
  }

}



String ESPboyOTA2::getWiFiStatusName() {
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



RESULT_STATE ESPboyOTA2::get_mess(){
 String resp="";
 char *pch;
  
  resp = GETmethod(GET_MESS);
  pch = strtok ((char *)resp.c_str(),";");
  
  if(pch[0]!='0') return(FAULT_STATE);

  for (uint8_t i=0; i<5; i++){
    pch = strtok (NULL,";");
    messStr[i].messmess = pch; 
    pch = strtok (NULL,";");
    messStr[i].messcolor = atoi(pch);
    pch = strtok (NULL,";");
    messStr[i].messdelay = atol(pch);
  }
  
  return (OK_STATE);
};

  
RESULT_STATE ESPboyOTA2::get_user(){
 String resp="";
 char *pch;

  resp = GETmethod(GET_USER); 
  pch = strtok ((char *)resp.c_str(),";");

  if(pch[0]=='0'){
    pch = strtok (NULL,";");
    globalStr->nicknameUser = pch;
    pch = strtok (NULL,";");
    globalStr->accessUser = atoi(pch);
    pch = strtok (NULL,";");
    globalStr->userLineNo = atoi(pch);
    pch = strtok (NULL,";");
    globalStr->lastVisitTimestamp = atoi(pch);
    globalStr->appLineNo = 0;
    return(OK_STATE); 
  }

  if(pch[0]=='2' || pch[0]=='3'){
    globalStr->nicknameUser = F("Unknown");
    globalStr->accessUser = 0;
    globalStr->userLineNo = 0;
    return(OK_STATE);
  }

  return(FAULT_STATE);
};


RESULT_STATE ESPboyOTA2::get_type(){
 String resp="";
 char *pch;
 uint16_t itemsQuant;
 uint16_t selectedItem;
 String toPrint;

  terminalGUIobj->printConsole(F(""), TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole(F("TYPEs update"), TFT_MAGENTA, 1, 0);
  
  resp = GETmethod(GET_TYPE);
  pch = strtok ((char *)resp.c_str(),";");
  if(pch[0]!='0') return(FAULT_STATE);

  pch = strtok (NULL,";");
  itemsQuant = atoi(pch);
  
  for (uint16_t i=0; i<itemsQuant; i++){
    iList.push_back(iL());
    pch = strtok (NULL,";");
    iList[i].iItem = pch;
    pch = strtok (NULL,";");
    iList[i].iData = atoi(pch); 
  }

  terminalGUIobj->printConsole(F("TYPE:"), TFT_GREEN, 1, 1);
  
  for (uint16_t i=0; i<itemsQuant; i++){
    toPrint = (String)(i+1);
    if(globalStr->lastVisitTimestamp < iList[i].iData && globalStr->userLineNo) toPrint += '!';
    else toPrint += ' ';
    toPrint += iList[i].iItem;
    terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  }

  terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 1, 0);

  selectedItem = 0;
  while (selectedItem==0 || selectedItem>itemsQuant){ 
    selectedItem = terminalGUIobj->getUserInput().toInt();
  };

  globalStr->type = iList[selectedItem-1].iItem;
  terminalGUIobj->printConsole(globalStr->type, TFT_GREEN, 1, 1);

  iList.clear();  

  return (OK_STATE);
};

  

RESULT_STATE ESPboyOTA2::get_genre(){
 String resp;
 char *pch;
 uint16_t itemsQuant;
 int16_t selectedItem;
 String toPrint;
  
  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole("GENREs update", TFT_MAGENTA, 1, 0);

  resp = GETmethod(GET_GENRE);
  
  pch = strtok ((char *)resp.c_str(),";");
  if(pch[0]!='0') return(FAULT_STATE);

  pch = strtok (NULL,";");
  itemsQuant = atoi(pch);
  
  for (uint16_t i=0; i<itemsQuant; i++){
    iList.push_back(iL());
    pch = strtok (NULL,";");
    iList[i].iItem = pch;
    pch = strtok (NULL,";");
    iList[i].iData = atoi(pch); 
  }

  iList.push_back(iL());
  iList[itemsQuant].iItem = itemBck;
  iList[itemsQuant].iData = globalStr->lastVisitTimestamp;

  terminalGUIobj->printConsole(F("GENRE:"), TFT_GREEN, 1, 1);

  for (uint16_t i=0; i<iList.size(); i++){
    toPrint = (String)(i+1);
    if(globalStr->lastVisitTimestamp < iList[i].iData && globalStr->userLineNo) toPrint += '!';
    else toPrint += ' ';
    toPrint += iList[i].iItem;
    terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  }

  terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 1, 0);

  selectedItem = 0;
  while (selectedItem==0 || selectedItem>itemsQuant+1){ 
    selectedItem = terminalGUIobj->getUserInput().toInt();
  };
  
  globalStr->genre = iList[selectedItem-1].iItem;
  terminalGUIobj->printConsole(globalStr->genre, TFT_GREEN, 1, 1);

  iList.clear();  

  if(selectedItem>itemsQuant) return (BACK_STATE);
  else return (OK_STATE);
};



RESULT_STATE ESPboyOTA2::get_app(){
 
 String resp="";
 char *pch;
 uint16_t itemsQuant;
 int16_t selectedItem;
 String toPrint;
  
  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole("APPs update", TFT_MAGENTA, 1, 0);
  
  resp = GETmethod(GET_APP);
  
  pch = strtok ((char *)resp.c_str(),";");
  if(pch[0]!='0') return(FAULT_STATE);

  pch = strtok (NULL,";");
  itemsQuant = atoi(pch);

  for (uint16_t i=0; i<itemsQuant; i++){
    iList.push_back(iL());
    pch = strtok (NULL,";");
    iList[i].iItem = pch;
    pch = strtok (NULL,";");
    iList[i].iData = atoi(pch); 
    pch = strtok (NULL,";");
    iList[i].iAccsess = atoi(pch); 
    pch = strtok (NULL,";");
    iList[i].iLike = atoi(pch); 
  }

  iList.push_back(iL());
  iList[itemsQuant].iItem = itemBck;

  terminalGUIobj->printConsole(F("APP:"), TFT_GREEN, 1, 1);
  
  for (uint16_t i=0; i<iList.size(); i++){
    toPrint = (String)(i+1);
    if (globalStr->accessUser < iList[i].iAccsess) toPrint += '-';
    else
      if(globalStr->lastVisitTimestamp < iList[i].iData && globalStr->userLineNo) toPrint += '!';
        else toPrint += ' ';
    toPrint += iList[i].iItem;
    if(i<itemsQuant){
       String toPrint2; 
       toPrint2 = ' ' + (String)iList[i].iAccsess + '/' + (String)iList[i].iLike;
       toPrint = toPrint.substring(0, (128/GUI_FONT_WIDTH) - toPrint2.length() -1);
       toPrint += toPrint2;
    }
    terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  }

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);

backlabel:
  selectedItem=0;
  terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 1, 1);
  while (selectedItem==0 || selectedItem>itemsQuant+1){ 
    selectedItem = terminalGUIobj->getUserInput().toInt();
  };

  if (selectedItem!=itemsQuant+1 && globalStr->accessUser < iList[selectedItem-1].iAccsess){
     terminalGUIobj->printConsole(F("Low accsess level"), TFT_RED, 1, 1);
     delay(3000);
     goto backlabel;
   }

  globalStr->app = iList[selectedItem-1].iItem;
  terminalGUIobj->printConsole(globalStr->app, TFT_GREEN, 1, 1);

  iList.clear();

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);

  if(selectedItem>itemsQuant) return (BACK_STATE);
  else return (OK_STATE);
};



RESULT_STATE ESPboyOTA2::get_info(){
 String resp="";
 char *pch;
 uint8_t selectedItem;
 String toPrint;

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  
  resp = GETmethod(GET_INFO);  
 
  pch = strtok ((char *)resp.c_str(),";");
  if(pch[0]!='0') return(FAULT_STATE);
  
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Game name:"), TFT_GREEN, 1, 0);
  terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Author:"), TFT_GREEN, 1, 0);
  terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("License:"), TFT_GREEN, 1, 0);
  terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Description:"), TFT_GREEN, 1, 0);
  terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  globalStr->updlink = pch;
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Access level/likes:"), TFT_GREEN, 1, 0);
  toPrint = pch;
  pch = strtok (NULL,";");
  toPrint += '/';
  toPrint += pch;
  terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  globalStr->appLineNo = atoi(pch);
  

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole(finalMnu[0], TFT_YELLOW, 1, 0);
  toPrint = finalMnu[1];
  if (!globalStr->userLineNo) toPrint[1]='-';
  terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  terminalGUIobj->printConsole(finalMnu[2], TFT_YELLOW, 1, 0);

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  
backlabel2:
  terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 0, 1);
  selectedItem = 0;
  while (selectedItem!=1 && selectedItem!=2 && selectedItem!=3){ 
    selectedItem = terminalGUIobj->getUserInput().toInt();
  };

  if(selectedItem==2 && !globalStr->userLineNo){
    terminalGUIobj->printConsole(F("Low accsess level"), TFT_RED, 0, 1);
    delay(3000);
    goto backlabel2;
  }

  toPrint = finalMnu[selectedItem-1];
  toPrint = toPrint.substring(2);
  terminalGUIobj->printConsole(toPrint, TFT_GREEN, 1, 1);

  switch (selectedItem){
    case 1: return (OK_STATE); break;
    case 2: return (LIKE_STATE); break;
    case 3: return (BACK_STATE); break;
  };
  
};


void ESPboyOTA2::checkOTA() {
  String toPrint;

  toPrint = F("ESPboy AppStore v");
  terminalGUIobj->printConsole( toPrint + (String)OTAv, TFT_GREEN, 0, 0);
  terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
  
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.mode(WIFI_STA);

  while (!wifiConnect());
    
  messStr = new messStruct[5];
  globalStr = new globalStruct();

  terminalGUIobj->printConsole(F("Server check..."), TFT_MAGENTA, 0, 0);
  if (get_mess() == OK_STATE && get_user() == OK_STATE)
    terminalGUIobj->printConsole(F("Server OK"), TFT_MAGENTA, 0, 1);
  else{
    terminalGUIobj->printConsole(F("FAULT, try later"), TFT_RED, 0, 0);
    while (1) delay(1000);
  }

  terminalGUIobj->printConsole(F("Server init..."), TFT_MAGENTA, 0, 0);
  if (POSTmethod(POST_CONNECT) == "OK")
    terminalGUIobj->printConsole(F("Init OK"), TFT_MAGENTA, 0, 1);
  else{
    terminalGUIobj->printConsole(F("FAULT, try later"), TFT_RED, 0, 0);
    while (1) delay(1000);
  }
  

  terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);

  if (!globalStr->userLineNo){
    terminalGUIobj->printConsole(messStr[USERNOTFOUND_MESS].messmess, messStr[USERNOTFOUND_MESS].messcolor, 0, 0);
    toPrint = F("ID ");
    toPrint += ESP.getChipId();
    terminalGUIobj->printConsole(toPrint, messStr[USERNOTFOUND_MESS].messcolor, 0, 0);
    toPrint = F("MC ");
    toPrint += WiFi.macAddress();
    terminalGUIobj->printConsole(toPrint, messStr[USERNOTFOUND_MESS].messcolor, 0, 0);
    delay(messStr[USERNOTFOUND_MESS].messdelay);
  }
  else{
    terminalGUIobj->printConsole(messStr[USERFOUND_MESS].messmess, messStr[USERFOUND_MESS].messcolor, 0, 0);
    terminalGUIobj->printConsole(globalStr->nicknameUser, messStr[USERFOUND_MESS].messcolor, 0, 0);
    toPrint = F("Access level "); 
    toPrint += globalStr->accessUser;
    terminalGUIobj->printConsole(toPrint, messStr[USERFOUND_MESS].messcolor, 0, 0);
    
    toPrint = F("Last seen ");
    uint16_t yer = globalStr->lastVisitTimestamp/500;
    uint8_t mnt = (globalStr->lastVisitTimestamp - yer*500)/32;
    uint8_t dte = globalStr->lastVisitTimestamp - yer*500 - mnt*32;
    if (dte<10) toPrint+= '0';
    toPrint +=  (String)dte + ".";
    if (mnt<10) toPrint+= '0';
    toPrint += (String)mnt + "." + (String)yer;
    terminalGUIobj->printConsole(toPrint, messStr[USERFOUND_MESS].messcolor, 0, 0);
    delay(messStr[USERFOUND_MESS].messdelay);
  }

  terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
  terminalGUIobj->printConsole(messStr[START_MESS].messmess, messStr[START_MESS].messcolor, 0, 0);
  delay (messStr[START_MESS].messdelay);
    
  GET_cmd = GET_TYPE;
  
  while(1){
    RESULT_STATE recievedState;
    switch (GET_cmd){
      case GET_TYPE:
        if (get_type() == OK_STATE) GET_cmd = GET_GENRE;
      case GET_GENRE:
        recievedState = get_genre();
        if (recievedState == OK_STATE) GET_cmd = GET_APP;
        if (recievedState == BACK_STATE) GET_cmd = GET_TYPE;        
        break;      
      case GET_APP:
        recievedState = get_app();
        if (recievedState == OK_STATE) GET_cmd = GET_INFO;
        if (recievedState == BACK_STATE) GET_cmd = GET_GENRE;        
        break;      
      case GET_INFO:
        recievedState = get_info();
        if (recievedState == OK_STATE) GET_cmd = GET_UPDATE;
        if (recievedState == BACK_STATE) GET_cmd = GET_APP;  
        if (recievedState == LIKE_STATE) GET_cmd = GET_LIKE;        
        break;      
      case GET_LIKE:
        terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
        terminalGUIobj->printConsole(F("Update like..."), TFT_MAGENTA, 1, 0);
        POSTmethod(POST_LIKE);
        POSTmethod(POST_LOG_LIKE);
        terminalGUIobj->printConsole(F("OK"), TFT_GREEN, 1, 0);
        GET_cmd = GET_INFO;
        break;  
      case GET_UPDATE:
        terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
        terminalGUIobj->printConsole(messStr[BEFOREUPDATE_MESS].messmess, messStr[BEFOREUPDATE_MESS].messcolor, 1, 0);
        POSTmethod(POST_DOWNLOAD);
        delay(messStr[BEFOREUPDATE_MESS].messdelay);
        terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
        terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);        
        if (UPDATEmethod()) {
          terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
          terminalGUIobj->printConsole(messStr[UPDATEFINISHWD_MESS].messmess, messStr[UPDATEFINISHWD_MESS].messcolor, 1, 0);
          delay(messStr[UPDATEFINISHWD_MESS].messdelay);
          ESP.restart();
        }
        else {
          terminalGUIobj->printConsole(F("Download FAILED"), TFT_RED, 0, 0);
          delay(3000);
          GET_cmd = GET_INFO;}      
      break;         
      }
    }
}
