// Data Example: 19.5058, 0.062, 26 Jul 2015, 08:50:43
#ifndef _SBE39

#define _SBE39

#include "CTD.h"


class SBE39 : public CTD {

    public:

    SBE39() {
        newData = false;
        this->fmtString = "%f, %f, %d %3s &d, %d:%d:%d";
        echoData = true;
        bufferIndex = 0;
        reading = false;
    }

    bool parseData(char * data) {
        newData = false;
        float c, t, d, sec;
        char mon[4];
        int year, day, hour, min;
        int res = sscanf(data, fmtString, &t, &d, &day, &mon, &year, &hour, &min, &sec);

        if (res == 8) {
            dBar = d;
            cond = c;
            temp = t;
            lastHour = hour;
            lastMinute = min;
            lastSecond = (int)sec;
            lastYear = year;
            lastMonth = toDecimalMonth(mon);
            lastDay = day;
            newData = true;
        }

        return newData;
    }

    private:

    int toDecimalMonth(const char * mon) {

        int decMon = 0;

        if (mon != NULL && strlen(mon) == 3) {
            
            // Unique by first character
            if (tolower(mon[0]) == 'f')
                decMon = 2; // February
            else if (tolower(mon[0]) == 's')
                decMon = 9; // September
            else if (tolower(mon[0]) == 'o')
                decMon = 10; // October
            else if (tolower(mon[0]) == 'n')
                decMon = 11; // November
            else if (tolower(mon[0]) == 'd')
                decMon = 12; // December
            
            // Unique with two chars
            else if (tolower(mon[0]) == 'j') {
                if (tolower(mon[1]) == 'a')
                    decMon = 1; // January
                else if (tolower(mon[2]) == 'n')
                    decMon = 6; // June
                else 
                    decMon = 7; // July
            }
            else if (tolower(mon[0]) == 'm') {
                if (tolower(mon[2]) == 'r')
                    decMon = 3; // March
                else 
                    decMon = 5; // May
            }
            else if (tolower(mon[0]) == 'a') {
                if (tolower(mon[1]) == 'p')
                    decMon = 4; // April
                else 
                    decMon = 8; // Aug
            }
        }

        return decMon;

    }

};

#endif
