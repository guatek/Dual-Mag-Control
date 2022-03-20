#ifndef _RBR

#define _RBR

#include <Arduino.h>
#include "Config.h"

#define MAX_BUFFER_LENGTH 256

class RBRInstrument {

    private:
    float dBar;
    float temp;
    float cond;
    bool newData;
    bool echoData;
    int lastHour, lastMinute, lastSecond, lastYear, lastMonth, lastDay;
    char buffer[MAX_BUFFER_LENGTH];
    int bufferIndex;
    volatile bool reading;


    public:

    RBRInstrument() {
        newData = false;
        echoData = true;
        bufferIndex = 0;
        reading = false;
    }

    bool parseData(char * data) {
        newData = false;
        float c, t, d, sec;
        int year, mon, day, hour, min;
        int res = sscanf(data, "%d-%d-%d %d:%d:%f,%f,%f,%f", &year, &mon, &day, &hour, &min, &sec, &c, &t, &d);

        if (res != 9) {
            // try the t/d version
            int res = sscanf(data, "%d-%d-%d %d:%d:%f,%f,%f", &year, &mon, &day, &hour, &min, &sec, &t, &d);
            if (res == 8) {
                dBar = d;
                temp = t;
                lastHour = hour;
                lastMinute = min;
                lastSecond = (int)sec;
                lastYear = year;
                lastMonth = mon;
                lastDay = day;
                newData = true;

            }
        }
        else {
            dBar = d;
            cond = c;
            temp = t;
            lastHour = hour;
            lastMinute = min;
            lastSecond = (int)sec;
            lastYear = year;
            lastMonth = mon;
            lastDay = day;
            newData = true;
        }

        return newData;
    }

    void readData(Stream * port) {
        
        if (port != NULL && port->available()) {
            reading = true;
            int bytesAvail = port->available();
            if (bufferIndex + bytesAvail < MAX_BUFFER_LENGTH - 1) { // -1 to give space for null term char
                char c;
                for (int i = 0; i < bytesAvail; i++) {
                    c = port->read();
                    if (c == '\n' || c == '\r') {
                        if (bufferIndex > 0) {
                            buffer[bufferIndex++] = '\0';
                            parseData(buffer);
                            bufferIndex = 0;
                            if (echoData) {
                                UI1.println(buffer);
                            }
                            
                        }
                    }
                    else {
                        buffer[bufferIndex++] = c;
                    }
                }
            }
            reading = false;
        }
    }

    void setEchoData(bool echo) {
        echoData = echo;
    }

    float temperature() {
        return temp;
    }

    float pressure() { 
        return dBar;
    }

    float conductivity() {
        return cond;
    }

    void getTimeString(char * buffer) {
        sprintf(buffer,"%04d-%02d-%02dT%02d:%02d:%02d",
            lastYear,
            lastMonth,
            lastDay,
            lastHour,
            lastMinute,
            lastSecond
        );
    }

    bool haveNewData() {
        return newData;
    }

    bool isReading() {
        return reading;
    }

    void invalidateData() {
        newData = false;
    }

    void enableEcho() {
        echoData = true;
    }

    void disableEcho() { 
        echoData = false;
    }
};

#endif
