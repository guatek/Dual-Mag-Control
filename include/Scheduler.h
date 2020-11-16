#ifndef _SCHEDULER

#define SCHEDULER

#include <Arduino.h>
#include <RTCZero.h>

#define MAX_TIME_EVENTS
#define MAX_DEPTH_EVENTS

class TimeEvent {
    public:
    int hour, min, sec, duration;
    uint32_t startTime;
    bool running, completed;

    TimeEvent(int hours, int min, int sec, int duration) {
        this->hour = hours;
        this->min = min;
        this->sec = sec;
        this->duration = duration;
        running = false;
        completed = false;
        startTime = 0;


    }

    bool checkStart(uint8_t h, uint8_t m, uint8_t s, RTCZero * rtc) {
        if (!running && (this->hour == h && this->min == m && this->sec <= s)) {
            running = true;
            startTime = rtc->getEpoch();
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool checkEnd(RTCZero * rtc) {
        if (running && (rtc->getEpoch() - startTime)/60 > duration) {
            running = false;
            return true;
        }
        else {
            return false;
        }
    }
};

class DepthEvent {
    public:
    float minDepth, maxDepth;
    bool running;

    DepthEvent(float minDepth, float maxDepth) {
        this->minDepth = minDepth;
        this->maxDepth = maxDepth;
        running = false;
    }

    bool checkStart(float depth) {
        if (!running && minDepth < depth && depth < maxDepth) {
            running = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool checkEnd(float depth) {
        if (running && minDepth < depth && depth < maxDepth) {
            running = false;
            return true;
        }
        else {
            return false;
        }
    }
};

class Scheduler {
    private:
};

#endif