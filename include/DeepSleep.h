#ifndef _DEEPSLEEP

#define _DEEPSLEEP
#include <Arduino.h>
#include <RTCZero.h>

class DeepSleep {

    public:

    DeepSleep() {
    }

    void goToSleep(uint8_t sec, uint8_t min, uint8_t hour, RTCZero * rtc, RTCZero::Alarm_Match alarmMatch) {
        if (rtc != NULL) {
            rtc->setAlarmTime(0, 0, 0);
            rtc->enableAlarm(alarmMatch);
            rtc->standbyMode();
        }
    }

};


#endif
