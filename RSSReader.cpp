/*
 *  RSSReader.h - Header file for RSSReader Class
 *
 *  Concept, Design and Implementation by: Craig A. Lindley
 *  Last Update: 11/06/2015
 */

#include <ESP8266WiFi.h>
#include "RSSReader.h"

#define HTTPPORT 80

enum STATES {INIT, START, READ_DATE, READ_TITLE, READ_DESCR} state=INIT;
enum STATES_CLOSE {INIT_CLOSE, START_CLOSE} stateClose=INIT_CLOSE;

enum TGS {DTA_TAG, TITLE_TAG, DESC_TAG, CLOSE_DTA_TAG, CLOSE_TITLE_TAG, CLOSE_DESC_TAG};
const String tags[]={"pubDate", "title", "description", "/pubDate", "/title", "/description"};

bool RSSReader::runStateMachine(char ch) {
 static String tagReaded;
 static String infoReaded;
 uint16_t stopFlag;
 bool terminateReadFlag=false;

  switch (state) {
    case INIT:
      if(ch=='<'){
        state=START;
        infoReaded=""; 
        tagReaded="";
      }
      break;
    case START:
      if(ch!='>') tagReaded+=ch;
      else {
         //Serial.println("");
         //Serial.println("DETECTED OPEN "+ tagReaded);
         if (tagReaded == tags[DTA_TAG]) {state=READ_DATE; tagReaded=""; break;}
         if (tagReaded == tags[TITLE_TAG]) {state=READ_TITLE; tagReaded=""; break;}
         if (tagReaded == tags[DESC_TAG]) {state=READ_DESCR; tagReaded=""; break;}
         state=INIT;
      }
      break;
    case READ_DATE:
      stopFlag=checkStop(ch);
      if(!stopFlag) infoReaded+=ch;
      else {
        infoReaded=infoReaded.substring(0, infoReaded.length()-(stopFlag));
        infoReaded = clearStr(infoReaded);
        infoReaded = clearStrTime(infoReaded);
        if (dateCallback(infoReaded.c_str())) terminateReadFlag=true;
        state=INIT;
      };
      break;
    case READ_TITLE:
      stopFlag=checkStop(ch);
      if(!stopFlag) infoReaded+=ch;
      else {
        infoReaded=infoReaded.substring(0, infoReaded.length()-(stopFlag));
        infoReaded = clearStr(infoReaded);
        if(titleCallback(infoReaded.c_str())) terminateReadFlag=true;
        state=INIT;
      };
      break;
    case READ_DESCR:
      stopFlag=checkStop(ch);
      if(!stopFlag) infoReaded+=ch;
      else {
        infoReaded=infoReaded.substring(0, infoReaded.length()-(stopFlag));
        infoReaded = clearStr(infoReaded);
        if(descCallback(infoReaded.c_str())) terminateReadFlag=true;
        state=INIT;
      }
      break;
  }
  return(terminateReadFlag);
}



uint16_t RSSReader::checkStop(char ch){
 static String tagReadedClose;
 uint16_t returnFlag=0;

 if (ch=='&') returnFlag=1;
 else{
  switch (stateClose){
    case INIT_CLOSE:
      if(ch=='<') {
        stateClose=START_CLOSE; 
        tagReadedClose="";}
      break;
    case START_CLOSE:
      if(ch!='>') {
        tagReadedClose+=ch;
        if(tagReadedClose=="!"){stateClose=INIT_CLOSE; returnFlag=0;};
        if (tagReadedClose=="div") {stateClose=INIT_CLOSE; returnFlag=3;};  
      }
      else{ 
        //Serial.println("");
        //Serial.println("DETECTED CLOSE "+ tagReadedClose);
        if(tagReadedClose==tags[CLOSE_DTA_TAG] ||
           tagReadedClose==tags[CLOSE_TITLE_TAG] ||
           tagReadedClose==tags[CLOSE_DESC_TAG]){
             returnFlag=tagReadedClose.length();}
         stateClose=INIT_CLOSE;  
       }
       break;   
   }
  }
 return(returnFlag);
};



RSSReader::RSSReader(int _timeoutInMS) {
  timeoutInMS = _timeoutInMS;
}


String RSSReader::clearStr(String str) {
  str.replace("CDATA", "");
  str.replace("[", "");
  str.replace("]", "");
  str.replace("!", "");
  str.replace(">", "");
  str.replace("<", "");
  str.replace("&#39", "");
  str.trim();
  return(str);
}

String RSSReader::clearStrTime(String str) {
  str.replace("GMT", "");
  str.replace("+0000", "");
  str.trim();
  return(str);
}


void RSSReader::setPubDateCallback(pt2Function _dateCallback) {
  dateCallback = _dateCallback;
}

void RSSReader::setTitleCallback(pt2Function _titleCallback) {
  titleCallback = _titleCallback;
}

void RSSReader::setDescCallback(pt2Function _descCallback) {
  descCallback = _descCallback;
}

// Parse URL and return result
bool RSSReader::parseURL(const char* url) {
  int index;
  const char *ptr;
  memset(host, 0, sizeof(host));
  memset(path, 0, sizeof(path));
  if ((ptr = strchr(url, ':')) != NULL)  {
    ptr += 3;
  } else  {
    ptr = url;
  }

  char *endPtr = strchr(ptr, '/');
  if (endPtr == NULL) {
    return false;
  }

  index = 0;
  while (ptr != endPtr) {
    host[index++] = *ptr++;
  }
  
  strcpy(path, endPtr);
  return true;
}



bool RSSReader::read(const char *url) {
  WiFiClient client;
  if (!parseURL(url)) {
    return false;
  }

  state = INIT;

  while (!client.connect(host, HTTPPORT)) {delay(200);}
  
  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: keep-alive" + "\r\n\r\n");

  unsigned long timeoutMS = timeoutInMS + millis();
  while (timeoutMS > millis()) {
    int count = client.available();
    while (count--) {
      char ch = client.read();
      //Serial.print(ch);
      if(runStateMachine(ch)){
        client.stop();
        return true;
      }
      timeoutMS = timeoutInMS + millis();
      yield();
    }
    delay(250);
  }
  client.stop();
  return false;
}
