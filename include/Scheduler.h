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
    int enabled;
    int flashType, lowMag, highMag, frameRate;
    uint32_t startTime;
    bool running, completed;

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
        enabled = 1;
        running = false;
        completed = false;
        startTime = 0;
    }

    int sizeOnFlash() {
        int size = 0;
        size += sizeof(hour) + sizeof(min) + sizeof(sec) + sizeof(duration);
        size += sizeof(flashType) + sizeof(lowMag) + sizeof(highMag) + sizeof(frameRate);
        return size;
    }

    void writeToFlash(SPIFlash * _f) {
        if (_f != NULL) {
            // erase block first so we can write to it
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
            _f->writeBytes(add, (void*)&enabled, (uint16_t)sizeof(enabled));
            add += sizeof(enabled);
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
            _f->writeBytes(add, (void*)&enabled, (uint16_t)sizeof(enabled));
            add += sizeof(enabled);
        }
    }

    bool checkStart(RTCZero * rtc) {
        int h,m,s;
        h = rtc->getHours();
        m = rtc->getMinutes();
        s = rtc->getSeconds();
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

    void printEvent(Stream * ui) {
        
        
        ui->print("\nTime Event [");
        ui->print(uid);
        ui->println("]:");
        ui->print("Start Hour: ");
        ui->println(hour);
        ui->print("Start Minute: ");
        ui->println(min);
        ui->print("Start Second: ");
        ui->println(sec);
        ui->print("Duration: ");
        ui->print(duration);
        ui->println(" minutes");
        ui->print("Flash Type: ");
        if (flashType)
            ui->println("Far Red");
        else
            ui->println("White");
        ui->print("Low Mag Duration: ");
        ui->print(lowMag);
        ui->println(" us");
        ui->print("high Mag Duration: ");
        ui->print(highMag);
        ui->println(" us");
        ui->print("Frame Rate: ");
        ui->print(frameRate);
        ui->println(" Hz");
        ui->print("Enabled?: ");
        if (enabled == 1)
            ui->println("ENABLED");
        else
            ui->println("DISABLED");

    }

    bool checkEnd(RTCZero * rtc) {
        if (running && (rtc->getEpoch() - startTime)/60 >= (unsigned long)duration) {
            running = false;
            return true;
        }
        else {
            return false;
        }
    }

    void setEnabled(bool e) {
        if (e) {
            enabled = 1;
        }
        else {
            enabled = 0;
        }
    }

    bool isEnabled() {
        return enabled == 1;
    }
};

class Scheduler {
    private:
    
    TimeEvent * timeEvents[MAX_TIME_EVENTS];
    int nTimeEvents;
    int baseUid, uid; // the base uid for the scheduler
    SPIFlash * _f;

    public:

    int flashType, lowMagDuration, highMagDuration, frameRate;
    
    Scheduler(int uid, SPIFlash * _f) {
        this->baseUid = uid;
        this->uid = uid + sizeof(this->baseUid);
        nTimeEvents = 0;
        this->_f = _f;

        // If _flash is not NULL, read the number of time and depth events
        // from flash and create as needed
        if (this->_f != NULL) {
            this->_f->readBytes(baseUid, (void*)&nTimeEvents, sizeof(nTimeEvents));
            DEBUGPORT.print("Read scheduler value: ");
            DEBUGPORT.println(nTimeEvents);
            if (nTimeEvents < 0 || nTimeEvents > MAX_TIME_EVENTS) {
                nTimeEvents = 0;
                DEBUGPORT.print("Scheduler: Error reading number of events, clearing");
            }
        }

        // Read in saved time events
        if (nTimeEvents > 0) {
            for (int i = 0; i < nTimeEvents; i++) {
                timeEvents[i] = new TimeEvent(this->uid, 0, 0, 0, 0, 0, 0, 0, 0); // dummy event to be filled from flash
                timeEvents[i]->readFromFlash(_f);
                uid += timeEvents[i]->sizeOnFlash();
                timeEvents[i]->printEvent(&DEBUGPORT);
            } 
        }

    }

    bool timeEventUI(Stream * ui, SystemConfig * cfg, int cmdTimeout) {
        int flashType, lowMagDuration, highMagDuration, frameRate; 
        bool result;
        char exitCode = 27;
        ui->println("Create New Time Event:");
        if (confirm(ui, "Use custom camera config? [y,N]: ", cmdTimeout)) {
            
            
            // Flash Type
            result = cfg->readIntFromUI(ui, FLASHTYPE, &flashType, exitCode, cmdTimeout);
            if (!result)
                return false;

            // Flash Duration
            if (flashType == 0) {
                // Color
                result = cfg->readIntFromUI(ui, LOWMAGCOLORFLASH, &lowMagDuration, exitCode, cmdTimeout);
                if (!result) 
                    return false;
                result = cfg->readIntFromUI(ui, HIGHMAGCOLORFLASH, &highMagDuration, exitCode, cmdTimeout);
                if (!result) 
                    return false;
            }
            else {
                // Far Red
                result = cfg->readIntFromUI(ui, LOWMAGREDFLASH, &lowMagDuration, exitCode, cmdTimeout);
                if (!result) 
                    return false;
                result = cfg->readIntFromUI(ui, HIGHMAGREDFLASH, &highMagDuration, exitCode, cmdTimeout);
                if (!result) 
                    return false;
            }
            
            // Frame rate
            result = cfg->readIntFromUI(ui, FRAMERATE, &frameRate, exitCode, cmdTimeout);
            if (!result)
                return false;

        }
        else {
            flashType = cfg->getInt(FLASHTYPE);
            if (flashType == 0) {
                lowMagDuration = cfg->getInt(LOWMAGCOLORFLASH);
                highMagDuration = cfg->getInt(HIGHMAGCOLORFLASH);
            }
            else {
                lowMagDuration = cfg->getInt(LOWMAGREDFLASH);
                highMagDuration = cfg->getInt(HIGHMAGREDFLASH);
            }
            frameRate = cfg->getInt(FRAMERATE);
        }

        int hour, minute, second, duration;

        // Setup dummy params to get hours, min, sec
        ConfigParam <int> p_hour("STARTHOUR", "The hour to start the event", "hours", 0, 0, 23, 0, false);
        result = p_hour.readFromCLI(ui,&hour, 27, cmdTimeout);
        if (!result)
            return false;

        ConfigParam <int> p_min("STARTMIN", "The minute to start the event", "min", 0, 0, 59, 0, false);
        result = p_min.readFromCLI(ui,&minute, 27, cmdTimeout);
        if (!result)
            return false;

        ConfigParam <int> p_sec("STARTSEC", "The second to start the event", "seconds", 0, 0, 59, 0, false);
        result = p_sec.readFromCLI(ui,&second, 27, cmdTimeout);
        if (!result)
            return false;

        ConfigParam <int> p_dur("DURATION", "The duration of the event in minutes", "minutes", 0, 5, 1440, 5, false);
        result = p_dur.readFromCLI(ui,&duration, 27, cmdTimeout);
        if (!result)
            return false;

        // If we got here we have a valid set of event params so we should create one.
        ui->println("Creating Event:");
        addTimeEvent(ui, hour, minute, second, duration, flashType, lowMagDuration, highMagDuration, frameRate);
        
        return true;

    }

    bool addTimeEvent(Stream * ui, int hour, int min, int sec, int duration, int flashType, int lowMag, int highMag, int frameRate) {
        if (nTimeEvents < MAX_TIME_EVENTS) {
            timeEvents[nTimeEvents] = new TimeEvent(uid, hour, min, sec, duration, flashType, lowMag, highMag, frameRate);
            timeEvents[nTimeEvents]->printEvent(ui);
            uid += timeEvents[nTimeEvents]->sizeOnFlash();
            nTimeEvents += 1;

            return true;
        }
        else {
            return false;
        }
    }

    void writeToFlash(Stream * in) {
        // Read in saved time events
        // Write the number of events on flash
        _f->writeBytes(baseUid, (void*)&nTimeEvents, sizeof(nTimeEvents));
        for (int i = 0; i < nTimeEvents; i++) {
            in->print("writing time event: ");
            in->print(timeEvents[i]->uid);
            in->println(" to flash...");
            timeEvents[i]->writeToFlash(_f);
        } 
    }

    bool setTimeEvent(int index, bool enabled) {
        if (index >= 0 && index < nTimeEvents) {
            timeEvents[index]->setEnabled(enabled);
            timeEvents[index]->writeToFlash(_f);
            return true;
        }
        else {
            return false;
        }
    }

    void printEvents(Stream * ui) {
        for (int i = 0; i < nTimeEvents; i++) {
            timeEvents[i]->printEvent(ui);
        } 
    }

    void clearEvents() {
        nTimeEvents = 0;
    }

    int checkEvents(RTCZero * rtc) {
        for (int i = 0; i < nTimeEvents; i++) {
            if (timeEvents[i]->checkStart(rtc)) {
                // store current camera config and set from event
                flashType = timeEvents[i]->flashType;
                lowMagDuration = timeEvents[i]->lowMag;
                highMagDuration = timeEvents[i]->highMag;
                frameRate = timeEvents[i]->frameRate;
                return 1;
            }
            if (timeEvents[i]->checkEnd(rtc)) {
                return -1;
            }
        }
        return 0;
    }
    
};

#endif