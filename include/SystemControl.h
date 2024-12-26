#ifndef _SYSTEMCONTROL

#define _SYSTEMCONTROL

#include <Arduino.h>
#include <RTCZero.h>
#include <RTCLib.h>
#include <WDTZero.h>
#include "Config.h"
#include "DeepSleep.h"
#include "SPIFlash.h"
#include "Sensors.h"
#include "Stats.h"
#include "Scheduler.h"
#include "SystemConfig.h"
#include "SystemTrigger.h"
#include "RBRInstrument.h"
#include "SBE39.h"
#include "Utils.h"

#define CMD_CHAR '!'
#define PROMPT "SPCVS01 > "
#define LOG_PROMPT "$SPCVS"
#define CMD_BUFFER_SIZE 128



// Global Sensors
Sensors _sensors;

// Global RTCZero
RTCZero _zerortc;

//Global RTCLib
RTC_DS3231 _ds3231;

// Global watchdog timer with 8 second hardware timeout
WDTZero _watchdog;

// RBR instrument
RBRInstrument _rbr;

// SBE39 CTD
SBE39 _sbe39;

bool uvcEnable = false;
int uvcDuration = 0;

void doseUVC() {
    if (!uvcEnable)
        return;
    digitalWrite(UVC_ENABLE,HIGH);
    delayMicroseconds(uvcDuration);
    digitalWrite(UVC_ENABLE,LOW);

}

class SystemControl
{
    private:
    float lastDepth;
    float currentDepth;
    bool systemOkay;
    bool ds3231Okay;
    bool cameraOn;
    bool pendingPowerOff;
    bool pendingPowerOn;
    bool lowVoltage;
    bool badEnv;
    char cmdBuffer[CMD_BUFFER_SIZE];
    bool rbrData;
    int state;
    unsigned long timestamp;
    unsigned long lastDepthCheck;
    unsigned long startupTimer;
    unsigned long lastPowerOnTime;
    unsigned long lastPowerOffTime;
    unsigned long pendingPowerOffTimer;
    unsigned long pendingPowerOnTimer;
    unsigned long clockSyncTimer;
    unsigned long envTimer;
    unsigned long voltageTimer;

    int lastFlashType, lastLowMagDuration, lastHighMagDuration, lastFrameRate;

    MovingAverage<float> avgVoltage;
    MovingAverage<float> avgTemp;
    MovingAverage<float> avgHum;
    MovingAverage<float> avgDepth;

    Scheduler * sch;
    
    void readInput(Stream *in) {
      
        if (in != NULL && in->available() > 0) {
            char c = in->read();
            if (c == CMD_CHAR) {

                // Don't echo the command char
                //if (cfg.getInt("LOCALECHO"))
                //    in->write(c);
              
                // Print the prompt
                in->write(PROMPT);


                unsigned long startTimer = millis();
                int index = 0;
                while (startTimer <= millis() && millis() - startTimer < (unsigned int)(cfg.getInt("CMDTIMEOUT"))) {

                    // Break if we have exceed the buffer size
                    if (index >= CMD_BUFFER_SIZE)
                        break;

                    // Wait on user input
                    if (in->available()) {
                        // Read the next char and reset timer          
                        c = in->read();
                        startTimer = millis();
                    }
                    else {
                        continue;
                    }

                    // Exit command loop on repeat command char
                    if (c == CMD_CHAR) {
                        break;
                    }
                    
                    if (c == '\r') {
                        // Command ended try to 
                        if (index <= CMD_BUFFER_SIZE)
                            cmdBuffer[index++] = '\0';
                        else
                            cmdBuffer[CMD_BUFFER_SIZE-1] = '\0';
                        
                        // Parse Command menus
                        char * rest;
                        char * cmd = strtok_r(cmdBuffer,",",&rest);

                        // CFG (configuration commands)
                        if (cmd != NULL && strncmp_ci(cmd,CFG, 3) == 0) {
                            if (rest != NULL) {
                                cfg.parseConfigCommand(rest, in);
                            }
                            else {
                                char timeString[64];
                                getTimeString(timeString);
                                cfg.printConfig(in, timeString);

                            }
                            
                        }

                        // PORTPASS (pass through to other serial ports)
                        if (cmd != NULL && strncmp_ci(cmd,PORTPASS, 8) == 0) {
                            doPortPass(in, rest);
                        }

                        // SETTIME (set time from string)
                        else if (cmd != NULL && strncmp_ci(cmd,SETTIME, 7) == 0) {
                            setTime(rest, in);
                        }

                        // WRITECONFIG (save the current config to EEPROM)
                        else if (cmd != NULL && strncmp_ci(cmd,WRITECONFIG, 11) == 0) {
                            writeConfig(in);
                            printAllPorts("Flash write completed.\n");
                        }

                        // READCONFIG (read the current config to EEPROM)
                        else if (cmd != NULL && strncmp_ci(cmd,READCONFIG, 10) == 0) {
                            readConfig(in);
                            printAllPorts("Flash read completed.\n");
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,CAMERAON,8) == 0) {
                            if (confirm(in, "Are you sure you want to power ON camera ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                turnOnCamera();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,CAMERAOFF,9) == 0) {
                            if (confirm(in, "Are you sure you want to power OFF camera ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                turnOffCamera();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,SHUTDOWNJETSON,14) == 0) {
                            if (confirm(in, "Are you sure you want to shutdown jetson ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                sendShutdown();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,NEWEVENT,8) == 0) {
                            sch->timeEventUI(in, &cfg, cfg.getInt(CMDTIMEOUT));
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,PRINTEVENTS,8) == 0) {
                            sch->printEvents(in);
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,CLEAREVENTS,8) == 0) {
                            if (confirm(in, "Are you sure you want clear all events ? [y,N]: ", cfg.getInt(CMDTIMEOUT)))
                                sch->clearEvents();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,GOTOSLEEP,9) == 0) {
                            goToSleep();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,PRINTPOWER,10) == 0) {
                            _sensors.printPower();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,UVCON,5) == 0) {
                            printAllPorts("Turning ON UVC...");
                            digitalWrite(UVC_ENABLE, HIGH);
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,UVCOFF,6) == 0) {
                            printAllPorts("Turning OFF UVC...");
                            digitalWrite(UVC_ENABLE, LOW);
                        }

                        // Reset the buffer and print out the prompt
                        if (c == '\n')
                            in->write('\r');
                        else
                            in->write("\r\n");

                        in->write(PROMPT);

                        index = 0;                       
                        startTimer = millis();
                        continue;
                    }
                    
                    // Handle backspace
                    if (c == '\b') {
                        index -= 1;
                        if (index < 0) {
                            index = 0;
                        }
                        else if ( index >= 0 && cfg.getInt(LOCALECHO)) {
                           in->write("\b \b");
                        }
                    }
                    else {
                        cmdBuffer[index++] = c;
                        if (cfg.getInt(LOCALECHO))
                            in->write(c);
                    }
                }
            }
        }
    }

    void doPortPass(Stream * in, char * cmd) {
        char * rest;
        char * num = strtok_r(cmd,",",&rest);
        in->print("Passing through to hardware port ");
        in->println(num);
        in->println();
        if (num != NULL) {
            char portNum = *num;
            switch (portNum) {
                case '0':
                    portpass(in, &HWPORT0, cfg.getInt(LOCALECHO) == 1);
                    break;
                case '1':
                    portpass(in, &HWPORT1, cfg.getInt(LOCALECHO) == 1);
                    break;
                case '2':
                    portpass(in, &HWPORT2, cfg.getInt(LOCALECHO) == 1);
                    break;
                case '3':
                    portpass(in, &HWPORT3, cfg.getInt(LOCALECHO) == 1);
                    break;
            }
        }
    }

    void setTime(char * timeString, Stream * ui) {
        if (timeString != NULL) {
            // if we have ds3231 set that first
            DateTime dt(timeString);
            if (dt.isValid()) {
                ui->println("\nUpdating clock...\n");
                if (ds3231Okay) {
                    _ds3231.adjust(dt.unixtime());
                }
                _zerortc.setEpoch(dt.unixtime());
            }
        }
    }
                      
 
    public:

    SystemConfig cfg;
    int trigWidth;
    int lowMagStrobeDuration;
    int highMagStrobeDuration;
    int violetStrobeDuration;
    int flashType;
    int frameRate;
  
    SystemControl() {
        systemOkay = false;
        rbrData = false;
        state = 0;
        timestamp = 0;
        ds3231Okay = false;
        pendingPowerOff = false;
        cameraOn = false;
        lowVoltage = false;
        badEnv = false;
    }

    bool begin() {


        // Start RTC
        _zerortc.begin();

        // Start DS3231
        ds3231Okay = true;
        if (!_ds3231.begin()) {
            DEBUGPORT.println("Could not init DS3231, time will be lost on power cycle.");
            ds3231Okay = false;
        }
        else {
            // sync rtczero to DS3231
            _zerortc.setEpoch(_ds3231.now().unixtime());
        }

        // set the startup timer
        startupTimer = _zerortc.getEpoch();
        lastPowerOffTime = _zerortc.getEpoch();
        lastPowerOnTime = _zerortc.getEpoch();
        lastDepthCheck = _zerortc.getEpoch();
        voltageTimer = _zerortc.getEpoch();
        envTimer = _zerortc.getEpoch();
        clockSyncTimer = _zerortc.getEpoch();

        lastDepth = -10.0;

        systemOkay = true;
        if (_flash.initialize()) {
            DEBUGPORT.println("Flash Init OK.");
        }
            
        else {
            DEBUGPORT.print("Init FAIL, expectedDeviceID(0x");
            DEBUGPORT.print(_expectedDeviceID, HEX);
            DEBUGPORT.print(") mismatched the read value: 0x");
            DEBUGPORT.println(_flash.readDeviceId(), HEX);
        }

        // Start sensors
        _sensors.begin();
        
        return true;

    }

    void storeLastFlashConfig() {
        // Set last config in case we call end event before start event
        lastFlashType = cfg.getInt(FLASHTYPE);
        lastFrameRate = cfg.getInt(FRAMERATE);
        if (lastFlashType == 1) {        
            lastLowMagDuration = cfg.getInt(LOWMAGREDFLASH);
            lastHighMagDuration = cfg.getInt(HIGHMAGREDFLASH);
        }
        else {
            lastLowMagDuration = cfg.getInt(LOWMAGCOLORFLASH);
            lastHighMagDuration = cfg.getInt(HIGHMAGCOLORFLASH);
        }
    }

    void restoreLastFlashConfig() {
        cfg.set(FLASHTYPE, lastFlashType);
        cfg.set(FRAMERATE, lastFrameRate);
        if (lastFlashType == 1) {        
            cfg.set(LOWMAGREDFLASH, lastLowMagDuration);
            cfg.set(HIGHMAGREDFLASH, lastHighMagDuration);
        }
        else {
            cfg.set(LOWMAGCOLORFLASH, lastLowMagDuration);
            cfg.set(HIGHMAGCOLORFLASH, lastHighMagDuration);
        }
    }

    void loadScheduler() {
        // Load scheduler
        sch = new Scheduler(SCHEDULER_UID, &_flash);
        storeLastFlashConfig();
    }

    void configWatchdog() {
        // enable hardware watchdog if requested
        if (cfg.getInt(WATCHDOG) > 0) {
            _watchdog.setup(WDT_HARDCYCLE8S);
        }
    }

    bool turnOnCamera() {
        //if (_zerortc.getEpoch() - lastPowerOffTime > (unsigned int)cfg.getInt(CAMGUARD) && !cameraOn) {
        DEBUGPORT.println("Turning ON camera power...");
        cameraOn = true;
        digitalWrite(V12_ENABLE, HIGH);
        digitalWrite(LED1_ENABLE, HIGH);
        digitalWrite(LED2_ENABLE, HIGH);
        digitalWrite(LED3_ENABLE, HIGH);
        lastPowerOnTime = _zerortc.getEpoch();
        return true;
        //}
        //else {
        //    return false;
        //}
    }

    bool turnOffCamera() {
        //if (_zerortc.getEpoch() - lastPowerOnTime > (unsigned int)cfg.getInt(CAMGUARD) && cameraOn) {
        DEBUGPORT.println("Turning OFF camera power...");
        cameraOn = false;
        digitalWrite(V12_ENABLE, LOW);
        digitalWrite(LED1_ENABLE, LOW);
        digitalWrite(LED2_ENABLE, LOW);
        digitalWrite(LED3_ENABLE, LOW);
        lastPowerOffTime = _zerortc.getEpoch();
        return true;
        //}
        //else {
        //    return false;
        //}
    }

    void getTimeString(char * timeString) {
        
        sprintf(timeString,"%s","YYYY-MM-DD hh:mm:ss");
        if (ds3231Okay) {
            DateTime now = _ds3231.now();
            now.toString(timeString);
        }
        else {
            sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d", 
                _zerortc.getYear(),
                _zerortc.getMonth(),
                _zerortc.getDay(),
                _zerortc.getHours(),
                _zerortc.getMinutes(),
                _zerortc.getSeconds()
            );
        }
    }

    bool update() {

        // Run updates and check for new data
        _sensors.update();

        // Build log string and send to UIs
        char output[256];

        float d = -1.0;
        currentDepth = d;

        char timeString[64];
        getTimeString(timeString);

        if (cfg.getInt(ECHORBR) == 1)
            _rbr.setEchoData(true);
        else
            _rbr.setEchoData(false);


        // The system log string, note this requires enabling printf_float build
        // option work show any output for floating point values
        sprintf(output, "%s01,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f",

            LOG_PROMPT,
            _sensors.temperature, // In C
            _sensors.pressure / 1000, // in kPa
            _sensors.humidity, // in %
            _sensors.voltage[0] / 1000, // In Volts
            _sensors.voltage[1] / 1000, // In Volts
            _sensors.voltage[2] / 1000, // In Volts
            _sensors.voltage[6] / 1000, // In Volts
            _sensors.voltage[3] / 1000, // In Volts
            _sensors.voltage[4] / 1000, // In Volts
            _sensors.voltage[5] / 1000, // In Volts
            _sensors.power[0] / 1000, // in W
            _sensors.power[1] / 1000, // in W
            _sensors.power[2] / 1000, // in W
            _sensors.power[6] / 1000, // in W
            _sensors.power[3] / 1000, // in W
            _sensors.power[4] / 1000, // in W
            _sensors.power[5] / 1000 // in W
            
        );

        // Send output
        printAllPorts(output);

        return true;
    }

    void writeConfig(Stream * in) {
        if (systemOkay) {
            cfg.writeConfig(in);
            sch->writeToFlash(in);
        }
    }

    void readConfig(Stream * in) {
        if (systemOkay)
            cfg.readConfig(in);
    }

    void checkInput() {
        if (DEBUGPORT.available() > 0) {
            readInput(&DEBUGPORT);
        }
        if (UI1.available() > 0) {
            readInput(&UI1);
        }
        if (UI2.available() > 0) {
            readInput(&UI2);
        }

    }

    void printAllPorts(const char output[]) {
        UI1.println(output);
        UI2.println(output);
        DEBUGPORT.println(output);
    }

    void checkCameraPower() {

        // Check for power off flag
        if (pendingPowerOff && ((_sensors.power[0] < 1000) || (_zerortc.getEpoch() - pendingPowerOffTimer > (unsigned int)cfg.getInt(MAXSHUTDOWNTIME)))) {
            turnOffCamera();
            pendingPowerOff = false;
            return;
        }

        // Check depth range

        // Never turn on camera if voltage is too low or env sensors are bad
        if (lowVoltage || badEnv) {
            return;
        }

        // turn on power under the following conditions:
        // (1) another event requested power on
        // (2) profile mode is set to always on
        if (pendingPowerOn || (!cameraOn && cfg.getInt(PROFILEMODE) == (unsigned int)1)) {
            printAllPorts("Powering ON camera...");
            turnOnCamera();
            pendingPowerOn = false;
        }
    }

    void checkEnv() {
        if (_zerortc.getEpoch() - startupTimer <= (unsigned int)cfg.getInt(STARTUPTIME))
            return;


        // Update moving average of temperature
        float latestTemp = avgTemp.update(_sensors.temperature);
        float latestHum = avgHum.update(_sensors.humidity);
        //float latestVoltage = _sensors.voltage[0];

        // Make sure this check happens AFTER updating the average measurement, otherwise
        // the average will not be calculated properly
        if (_zerortc.getEpoch() - envTimer <= (unsigned int)cfg.getInt(CHECKINTERVAL))
            return;

        // Reset check timer
        envTimer = _zerortc.getEpoch();

        if (latestTemp > cfg.getInt(TEMPLIMIT)) {
            char output[64];
            sprintf(output,"Temperature %0.2f C exceeds limit of %0.2f C", latestTemp, (float)cfg.getInt(TEMPLIMIT));
            printAllPorts(output);
            badEnv = true;
            if (cameraOn) {
                printAllPorts("Shuting down camera...");
                sendShutdown();
            }
        }

        if (latestHum > cfg.getInt(HUMLIMIT)) {
            char output[64];
            sprintf(output,"Humidity %0.2f %% exceeds limit of %0.2f %%", latestHum, (float)cfg.getInt(HUMLIMIT));
            printAllPorts(output);
            badEnv = true;
            if (cameraOn) {
                printAllPorts("Shuting down camera...");
                sendShutdown();
            }
        }

        badEnv = false;
        
    }

    void checkVoltage() {

        if (_zerortc.getEpoch() - startupTimer <= (unsigned int)cfg.getInt(STARTUPTIME))
            return;

        // Update moving average of voltage
        float latestVoltage = avgVoltage.update(_sensors.voltage[0]);
        //float latestVoltage = _sensors.voltage[0];

        // Make sure this check happens AFTER updating the average measurement, otherwise
        // the average will not be calculated properly
        if (_zerortc.getEpoch() - voltageTimer <= (unsigned int)cfg.getInt(CHECKINTERVAL))
            return;
        
        // Reset check timer
        voltageTimer = _zerortc.getEpoch();

        if (latestVoltage < 6000.0) {
            // likely on USB power, note voltage is in mV
            return;
        }

        // If battery voltage is too low, notify and sleep
        // If the camera is running at this point, shut it down first
        if (latestVoltage < cfg.getInt(LOWVOLTAGE)) {
            char output[256];
            sprintf(output,"Voltage %f below threshold %d", latestVoltage, cfg.getInt(LOWVOLTAGE));
            printAllPorts(output);
            if (cameraOn) {
                sendShutdown();
            }
            if (cfg.getInt(STANDBY) == 1 && !cameraOn) {
                goToSleep();
            }
        }
    }

    void goToSleep() {
        
        printAllPorts("Going to sleep...");
        _zerortc.setAlarmTime(0, 0, 0);
        if (cfg.getInt(CHECKHOURLY) == 1) {
            printAllPorts("Alarm Set for 1 Hour");
            _zerortc.enableAlarm(RTCZero::MATCH_MMSS);
        }
        else {
            printAllPorts("Alarm Set for 1 Minute");
            _zerortc.enableAlarm(RTCZero::MATCH_SS);
        }
        if (cfg.getInt(STANDBY) == 1) {
            _zerortc.standbyMode();
        }
    }

    void checkEvents() {
        int result = sch->checkEvents(&_zerortc);
        if (result == 1 && !pendingPowerOn && !cameraOn) {
            // Store the current settings and set new ones
            storeLastFlashConfig();
            cfg.set(FLASHTYPE, sch->flashType);
            cfg.set(FRAMERATE, sch->frameRate);
            if (sch->flashType == 1) {
                cfg.set(LOWMAGREDFLASH, sch->lowMagDuration);
                cfg.set(HIGHMAGREDFLASH, sch->highMagDuration);
            }
            else {
                cfg.set(LOWMAGCOLORFLASH, sch->lowMagDuration);
                cfg.set(HIGHMAGCOLORFLASH, sch->highMagDuration);
            }
            configureFlashDurations();
            setTriggers();
            pendingPowerOn = true;
            pendingPowerOnTimer = _zerortc.getEpoch();
        }
        else if (result == -1 && !pendingPowerOff && cameraOn) {
            restoreLastFlashConfig();
            sendShutdown();
        }
    }

    void sendShutdown() {
        if (cameraOn) {
            DEBUGPORT.println("Sending to Jetson: sudo shutdown -h now");
            JETSONPORT.println("pkill -TERM PlanktonCam; sleep 4; sudo shutdown -h now\n");
            pendingPowerOff = true;
            pendingPowerOffTimer = _zerortc.getEpoch();
        }
        else {
            DEBUGPORT.println("Camera not powered on, not sending shutdown command");
        }
    }

    void configureFlashDurations() {
        // Set global delays for ISRs
        trigWidth = cfg.getInt(TRIGWIDTH);
        if (flashType == 0) {
            lowMagStrobeDuration = cfg.getInt(LOWMAGCOLORFLASH);
            highMagStrobeDuration = cfg.getInt(HIGHMAGCOLORFLASH);
            violetStrobeDuration = cfg.getInt(VIOLETFLASH);
        }
        else {
            lowMagStrobeDuration = cfg.getInt(LOWMAGREDFLASH);
            highMagStrobeDuration = cfg.getInt(HIGHMAGREDFLASH);
            violetStrobeDuration = cfg.getInt(VIOLETFLASH);
        }
    }

    void setTriggers() {
        frameRate = cfg.getInt(FRAMERATE); 
        configTriggers(cfg.getInt(FRAMERATE));
    }

    void setUVC() {
        float durationCalc;
        uvcEnable = cfg.getInt(UVCENABLE);
        durationCalc = (float)cfg.getInt(UVCDUTY) / 100.0 * 1000000.0 / cfg.getInt(UVCFREQ); 
        uvcDuration = (int)durationCalc;
        configUVC(cfg.getInt(UVCFREQ), doseUVC);
    }
        
};

#endif
