#ifndef _SCHEDULER

#define SCHEDULER

#include <Arduino.h>
#include <SPIflash.h>
#include <RTCZero.h>
#include "Config.h"
#include "SystemConfig.h"
#include "Utils.h"

#define MAX_TIME_EVENTS 16
#define MAX_DEPTH_EVENTS 16

class TimeEvent {
    public:
    int uid, hour, min, sec, duration;
    int flashType, lowMag, highMag, frameRate;
    uint32_t startTime;
    bool running, completed;
    CameraConfig * camCfg;

    TimeEvent(int uid, int hours, int min, int sec, int duration, int flashType, int lowMag, int highMag, int frameRate) {
        this->uid = uid;
        this->hour = hours;
        this->min = min;
        this->sec = sec;
        this->duration = duration;
        this->flashType = flashType;
        this->lowMag = lowMag;
        this->highMag = highMag;
        this->frameRate = frameRate;
        running = false;
        completed = false;
        startTime = 0;
    }

    int sizeOnFlash() {
        int size = 0;
        size += sizeof(hour) + sizeof(min) + sizeof(sec) + sizeof(duration);
        size += sizeof(flashType) + sizeof(minVal) + sizeof(maxVal) + sizeof(duration);
        return size;
    }

    void writeToFlash(SPIFlash * _f) {
        if (_f != NULL) {
            int add = uid;
            _f->writeBytes(add, (void*)&hour, (uint16_t)sizeof(hour));
            add += sizeof(hour);
            _f->writeBytes(add, (void*)&min, (uint16_t)sizeof(min));
            add += sizeof(min);
            _f->writeBytes(add, (void*)&sec, (uint16_t)sizeof(sec));
            add += sizeof(sec);
            _f->writeBytes(add, (void*)&duration, (uint16_t)sizeof(duration));
            add += sizeof(duration);
            _f->writeBytes(add, (void*)&flashType, (uint16_t)sizeof(flashType));
            add += sizeof(flashType);
            _f->writeBytes(add, (void*)&lowMag, (uint16_t)sizeof(lowMag));
            add += sizeof(lowMag);
            _f->writeBytes(add, (void*)&highMag, (uint16_t)sizeof(highMag));
            add += sizeof(highMag);
            _f->writeBytes(add, (void*)&frameRate, (uint16_t)sizeof(frameRate));
            add += sizeof(frameRate);
        }
    }

    void readFromFlash(SPIFlash * _f) {
        if (_f != NULL) {
            int add = uid;
            _f->readBytes(add, (void*)&hour, (uint16_t)sizeof(hour));
            add += sizeof(hour);
            _f->readBytes(add, (void*)&min, (uint16_t)sizeof(min));
            add += sizeof(min);
            _f->readBytes(add, (void*)&sec, (uint16_t)sizeof(sec));
            add += sizeof(sec);
            _f->readBytes(add, (void*)&duration, (uint16_t)sizeof(duration));
            add += sizeof(duration);
            _f->readBytes(add, (void*)&flashType, (uint16_t)sizeof(flashType));
            add += sizeof(flashType);
            _f->readBytes(add, (void*)&lowMag, (uint16_t)sizeof(lowMag));
            add += sizeof(lowMag);
            _f->readBytes(add, (void*)&highMag, (uint16_t)sizeof(highMag));
            add += sizeof(highMag);
            _f->readBytes(add, (void*)&frameRate, (uint16_t)sizeof(frameRate));
            add += sizeof(frameRate);
        }
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
    int uid;
    float minDepth, maxDepth;
    bool running;
    int flashType, lowMag, highMag, frameRate;

    DepthEvent(int uid, float minDepth, float maxDepth, CameraConfig * camCfg) {
        this->uid = uid;
        this->minDepth = minDepth;
        this->maxDepth = maxDepth;
        this->flashType = flashType;
        this->lowMag = lowMag;
        this->highMag = highMag;
        this->frameRate = frameRate;
        running = false;
    }

    int sizeOnFlash() {
        int size = 0;
        size += sizeof(minDepth) + sizeof(maxDepth) + sizeof(flashType) + sizeof(minVal) + sizeof(maxVal) + sizeof(duration);
        return size;
    }

    void writeToFlash(SPIFlash * _f) {
        if (_f != NULL) {
            int add = uid;
            _f->writeBytes(add, (void*)&minDepth, (uint16_t)sizeof(minDepth));
            add += sizeof(minDepth);
            _f->writeBytes(add, (void*)&maxDepth, (uint16_t)sizeof(maxDepth));
            add += sizeof(maxDepth);
            _f->writeBytes(add, (void*)&flashType, (uint16_t)sizeof(flashType));
            add += sizeof(flashType);
            _f->writeBytes(add, (void*)&lowMag, (uint16_t)sizeof(lowMag));
            add += sizeof(lowMag);
            _f->writeBytes(add, (void*)&highMag, (uint16_t)sizeof(highMag));
            add += sizeof(highMag);
            _f->writeBytes(add, (void*)&frameRate, (uint16_t)sizeof(frameRate));
            add += sizeof(frameRate);
        }
        }
    }

    void readFromFlash(SPIFlash * _f) {
        if (_f != NULL) {
            int add = uid;
            _f->readBytes(add, (void*)&minDepth, (uint16_t)sizeof(minDepth));
            add += sizeof(minDepth);
            _f->readBytes(add, (void*)&maxDepth, (uint16_t)sizeof(maxDepth));
            add += sizeof(maxDepth);
            _f->readBytes(add, (void*)&flashType, (uint16_t)sizeof(flashType));
            add += sizeof(flashType);
            _f->readBytes(add, (void*)&lowMag, (uint16_t)sizeof(lowMag));
            add += sizeof(lowMag);
            _f->readBytes(add, (void*)&highMag, (uint16_t)sizeof(highMag));
            add += sizeof(highMag);
            _f->readBytes(add, (void*)&frameRate, (uint16_t)sizeof(frameRate));
            add += sizeof(frameRate);
        }
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
    int uid; // the base uid for the scheduler
    SPIFlash * _flash;

    public:

    Scheduler(int uid, int mode, SPIFlash * _flash) {
        this->uid = uid;
        nTimeEvents = 0;
        nDepthEvents = 0;
        this->mode = mode;
        this->_flash = _flash;

        // If _flash is not NULL, read the number of time and depth events
        // from flash and create as needed
        if (_flash != NULL) {
            _flash->readBytes(uid, (void*)&nTimeEvents, sizeof(nTimeEvents));
            if (nTimeEvents < 0 || nTimeEvents > MAX_TIME_EVENTS) {
                nTimeEvents = 0;
                DEBUGPORT.print("Scheduler: Error reading number of events, clearing");
            }
            _flash->readBytes(uid, (void*)&nDepthEvents, sizeof(nDepthEvents));
            if (nDepthEvents < 0 || nDepthEvents > MAX_TIME_EVENTS) {
                nDepthEvents = 0;
                DEBUGPORT.print("Scheduler: Error reading number of events, clearing");
            }
        }

        // Read in saved time events
        int uidOffset = uid;
        if (nTimeEvents > 0) {
            for (int i = 0; i <= nTimeEvents; i++) {
                timeEvents[i] = new TimeEvent(uidOffset, 0, 0, 0, 0); // dummy event to be filled from flash
                timeEvents[i]->readFromFlash(_flash);
                uidOffset += timeEvents[i]->sizeOnFlash();
            } 
        }
        // Read in saved depth events
        if (nDepthEvents > 0) {
            for (int i = 0; i <= nDepthEvents; i++) {
                depthEvents[i] = new DepthEvent(uidOffset, 0.0, 0.0); // dummy event to be filled from flash
                depthEvents[i]->readFromFlash(_flash);
                uidOffset += depthEvents[i]->sizeOnFlash();
            } 
        }


    }

    void writeToFlash() {
        // Read in saved time events
        int uidOffset = uid;
        if (nTimeEvents > 0) {
            for (int i = 0; i <= nTimeEvents; i++) {
                timeEvents[i]->writeToFlash(_flash);
            } 
        }
        // Read in saved depth events
        if (nDepthEvents > 0) {
            for (int i = 0; i <= nDepthEvents; i++) {
                depthEvents[i]->writeToFlash(_flash);

            } 
        }
    }

    bool timeEventUI(Stream * ui, SystemConfig * cfg, cmdTimeout) {
        ui->println("Create Time Event:");
        if (confirm(ui, "Use custom camera config? [y,N]: ", cmdTimeout)) {
            int flashType, lowMagDuration, highMagDuration, frameRate;
            int result = cfg->readIntFromUI(ui, FLASHTYPE, &flashType, 27, cmdTimeout);
            if (!result)
                return false;
            if (flashType == 0) {
                int result = cfg->readIntFromUI(ui, LOWMAG&lowMagDuration, 27, cmdTimeout);
                if (!result) 
                return false;
            

        }


    }

    bool addTimeEvent(int uid, int hour, int min, int sec, int duration) {
        if (nTimeEvents < MAX_TIME_EVENTS) {
            timeEvents[nTimeEvents] = new TimeEvent(uid, hour, min, sec, duration);
            enabledTimeEvents[nTimeEvents++] = true;
            return true;
        }
        else {
            return false;
        }
    }

    bool addDepthEvent(int uid, float minDepth, float maxDepth) {
        if (nDepthEvents < MAX_TIME_EVENTS) {
            depthEvents[nDepthEvents] = new DepthEvent(uid, minDepth, maxDepth);
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