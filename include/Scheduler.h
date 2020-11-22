#ifndef _SCHEDULER

#define SCHEDULER

#include <Arduino.h>
#include <RTCZero.h>

#define MAX_TIME_EVENTS 16
#define MAX_DEPTH_EVENTS 16

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
    
    TimeEvent * timeEvents[MAX_TIME_EVENTS];
    bool enabledTimeEvents[MAX_TIME_EVENTS];

    DepthEvent * depthEvents[MAX_TIME_EVENTS];
    bool enabledDepthEvents[MAX_TIME_EVENTS];
    
    int nTimeEvents;
    int nDepthEvents;
    int mode; // 0 => off, 1 => check time events, 2 => check depth events, 3 => check time OR depth, 4 => check time AND depth

    public:

    Scheduler(int mode) {
        nTimeEvents = 0;
        nDepthEvents = 0;
        this->mode = mode;
    }

    bool addTimeEvent(int hour, int min, int sec, int duration) {
        if (nTimeEvents < MAX_TIME_EVENTS) {
            timeEvents[nTimeEvents] = new TimeEvent(hour, min, sec, duration);
            enabledTimeEvents[nTimeEvents++] = true;
            return true;
        }
        else {
            return false;
        }
    }

    bool addDepthEvent(float minDepth, float maxDepth) {
        if (nDepthEvents < MAX_TIME_EVENTS) {
            depthEvents[nDepthEvents] = new DepthEvent(minDepth, maxDepth);
            enabledDepthEvents[nDepthEvents++] = true;
        }
        else {
            return false;
        }
    }

    bool setTimeEvent(int index, bool enabled) {
        if (index >= 0 && index < nTimeEvents) {
            enabledTimeEvents[index] = enabled;
            return true;
        }
        else {
            return false;
        }
    }

    bool setDepthEvent(int index, bool enabled) {
        if (index >= 0 && index < nDepthEvents) {
            enabledDepthEvents[index] = enabled;
            return true;
        }
        else {
            return false;
        }
    }

    int checkEvents() {
        switch (mode) {
            case 0:
                return -1;
            case 1:
                
        }
    }
    
};

#endif