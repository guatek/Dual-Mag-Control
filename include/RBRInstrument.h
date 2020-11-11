#ifndef _RBR

#define RBR

#include <Arduino.h>

class RBRInstrument {

    private:
    float dBar;
    float temp;
    float cond;
    bool newData;


    public:

    RBRInstrument() {
        newData = false;
    }

    bool parseData(char * data) {
        float c, t, d, sec;
        int year, mon, day, hour, min;
        newData = false;
        int res = sscanf("%d-%d-%d %d:%d:%f,%f,%f,%f", data, &year, &mon, &day, &hour, &min, &sec, &c, &t, &d);

        if (res == EOF) {
            // try the t/d version
            int res = sscanf("%d-%d-%d %d:%d:%f,%f,%f,%f", data, &year, &mon, &day, &hour, &min, &sec, &c, &t, &d);
            if (res != EOF) {
                newData = true;
            }
        }
        else {
            newData = true;
        }

        return newData;
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
};

#endif
