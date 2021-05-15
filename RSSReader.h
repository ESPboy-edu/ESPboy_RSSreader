/*
 *  RSSReader.h - Header file for RSSReader Class
 *
 *  Concept, Design and Implementation by: Craig A. Lindley
 *  Last Update: 11/06/2015
 */

#ifndef RSSREADER_H
#define RSSREADER_H


// A function pointer for callback
typedef bool (*pt2Function)(const char *);

class RSSReader {

  public:
    RSSReader(int _timeoutInMS);

    void setTitleCallback(pt2Function _titleCallback);
    void setDescCallback(pt2Function _descCallback);
    void setPubDateCallback(pt2Function _dateCallback);

    bool read(const char *url);

  private:
    int timeoutInMS;
    char host[30];
    char path[100];

    pt2Function titleCallback;
    pt2Function descCallback;
    pt2Function dateCallback;

    String clearStr(String str);
    String clearStrTime(String str);
    bool parseURL(const char* url);
    bool runStateMachine(char ch);
    uint16_t checkStop(char ch);

};

#endif
