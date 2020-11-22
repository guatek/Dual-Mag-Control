#ifndef _RBR

#define RBR

#include <Arduino.h>
#include "Config.h"

#define MAX_BUFFER_LENGTH 256

class RBRInstrument {

    private:
    float dBar;
    float temp;
    float cond;
    bool newData;
    char buffer[MAX_BUFFER_LENGTH];
    int bufferIndex;


    public:

    RBRInstrument() {
        newData = false;
        bufferIndex = 0;
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
                newData = true;

            }
        }
        else {
            dBar = d;
            cond = c;
            temp = t;
            newData = true;
        }

        return newData;
    }

    bool readData(Stream * port) {
        
        if (port != NULL && port->available()) {

            int bytesAvail = port->available();
            if (bufferIndex + bytesAvail < MAX_BUFFER_LENGTH - 1) { // -1 to give space for null term char
                char c;
                for (int i = 0; i < bytesAvail; i++) {
                    c = port->read();
                    if (c == '\n' || c == '\r') {
                        if (bufferIndex > 0) {
                            buffer[bufferIndex++] = '\0';
                            UI1.println(buffer);
                            UI2.println(buffer);
                            parseData(buffer);
                            bufferIndex = 0;
                        }
                    }
                    else {
                        buffer[bufferIndex++] = c;
                    }
                }
            }
        }
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

    bool syncClock(RTCZero * rtc) {
        if (newData) {
        }
    }

    bool haveNewData() {
        return newData;
    }

    void invalidateData() {
        newData = false;
    }
};

#endif
