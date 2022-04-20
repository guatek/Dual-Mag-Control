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
#include "SystemConfig.h"
#include "Utils.h"

#define CMD_CHAR '!'
#define PROMPT "CV > "
#define LOG_PROMPT "$CV"
#define CMD_BUFFER_SIZE 128

// Global Sensors
Sensors _sensors;

// Global RTCZero
RTCZero _zerortc;

//Global RTCLib
RTC_DS3231 _ds3231;

// Global watchdog timer with 8 second hardware timeout
WDTZero _watchdog;

// Static polling function for instruments
int instrumentType = 0;
bool pollingEnable = false;
bool echoRBR = false;

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
                        if (cmd != NULL && strncmp_ci(cmd,SETTIME, 7) == 0) {
                            setTime(rest, in);
                        }

                        // WRITECONFIG (save the current config to EEPROM)
                        if (cmd != NULL && strncmp_ci(cmd,WRITECONFIG, 11) == 0) {
                            writeConfig();
                        }

                        // READCONFIG (read the current config to EEPROM)
                        if (cmd != NULL && strncmp_ci(cmd,READCONFIG, 10) == 0) {
                            readConfig();
                        }

                        if (cmd != NULL && strncmp_ci(cmd,CAMERAON,8) == 0) {
                            if (confirm(in, "Are you sure you want to power ON camera ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                turnOnCamera();
                        }

                        if (cmd != NULL && strncmp_ci(cmd,CAMERAOFF,9) == 0) {
                            if (confirm(in, "Are you sure you want to power OFF camera ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                turnOffCamera();
                        }

                        if (cmd != NULL && strncmp_ci(cmd,SHUTDOWNJETSON,14) == 0) {
                            if (confirm(in, "Are you sure you want to shutdown jetson ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                sendShutdown();
                        }

                        if (cmd != NULL && strncmp_ci(cmd,STROBEALL,9) == 0) {
                            strobeAllLEDS();
                        }

                        if (cmd != NULL && strncmp_ci(cmd,STROBE1,7) == 0) {
                            strobe1();
                        }
                        
                        if (cmd != NULL && strncmp_ci(cmd,STROBE2,7) == 0) {
                            strobe2();
                        }
                        
                        if (cmd != NULL && strncmp_ci(cmd,STROBE3,7) == 0) {
                            strobe3();
                        }
                        
                        if (cmd != NULL && strncmp_ci(cmd,STROBE4,7) == 0) {
                            strobe4();
                        }

                        if (cmd != NULL && strncmp_ci(cmd,LASERON,7) == 0) {
                            digitalWrite(LASER_ENABLE, HIGH);
                        }
                        if (cmd != NULL && strncmp_ci(cmd,LASEROFF,8) == 0) {
                            digitalWrite(LASER_ENABLE, LOW);
                        }

                        if (cmd != NULL && strncmp_ci(cmd,INDCOLORON,10) == 0) {
                            digitalWrite(INDCOLOR, HIGH);
                        }
                        if (cmd != NULL && strncmp_ci(cmd,INDCOLOROFF,11) == 0) {
                            digitalWrite(INDCOLOR, LOW);
                        }

                        if (cmd != NULL && strncmp_ci(cmd,INDMACROON,10) == 0) {
                            digitalWrite(INDMACRO, HIGH);
                        }
                        if (cmd != NULL && strncmp_ci(cmd,INDMACROOFF,11) == 0) {
                            digitalWrite(INDMACRO, LOW);
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

        ds3231Okay = false;

        // set the startup timer
        startupTimer = _zerortc.getEpoch();
        lastPowerOffTime = _zerortc.getEpoch();
        lastPowerOnTime = _zerortc.getEpoch();
        lastDepthCheck = _zerortc.getEpoch();
        voltageTimer = _zerortc.getEpoch();
        envTimer = _zerortc.getEpoch();
        clockSyncTimer = _zerortc.getEpoch();

        lastDepth = -10.0;

        if (_flash.initialize()) {
            DEBUGPORT.println("Flash Init OK.");
        }
           
        else {
            DEBUGPORT.print("Init FAIL, expectedDeviceID(0x");
            DEBUGPORT.print(_expectedDeviceID, HEX);
            DEBUGPORT.print(") mismatched the read value: 0x");
            DEBUGPORT.println(_flash.readDeviceId(), HEX);
        }

        systemOkay = true;

        // Start sensors
        _sensors.begin();
        
        return true;

    }

    void configWatchdog() {
        // enable hardware watchdog if requested
        if (cfg.getInt(WATCHDOG) > 0) {
            _watchdog.setup(WDT_HARDCYCLE8S);
        }
    }

    bool turnOnCamera() {
        if (_zerortc.getEpoch() - lastPowerOffTime > (unsigned int)cfg.getInt(CAMGUARD) && !cameraOn) {
            DEBUGPORT.println("Turning ON camera power...");
            cameraOn = true;
            digitalWrite(CAMERA_POWER, HIGH);
            lastPowerOnTime = _zerortc.getEpoch();
            return true;
        }
        else {
            return false;
        }
    }

    bool turnOffCamera() {
        if (_zerortc.getEpoch() - lastPowerOnTime > (unsigned int)cfg.getInt(CAMGUARD) && cameraOn) {
            DEBUGPORT.println("Turning OFF camera power...");
            cameraOn = false;
            digitalWrite(CAMERA_POWER, LOW);
            lastPowerOffTime = _zerortc.getEpoch();
            return true;
        }
        else {
            return false;
        }
    }

    void getTimeString(char * timeString) {
        
        sprintf(timeString,"%s","YYYY-MM-DD hh:mm:ss");
        unsigned long unixtime;
        if (ds3231Okay) {
            DateTime now = _ds3231.now();
            unixtime = now.unixtime();
            now.toString(timeString);
        }
        else {
            unixtime = _zerortc.getEpoch();
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

        float c = -1.0;
        float t = -1.0;
        float d = -1.0;
        currentDepth = d;

        uint32_t unixtime; 

        char timeString[64];
        getTimeString(timeString);

        // The system log string, note this requires enabling printf_float build
        // option work show any output for floating point values
        sprintf(output, "%s,%0.3f,%0.3f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%d,%d,%d",

            LOG_PROMPT,
            _sensors.temperature, // In C
            _sensors.pressure / 1000, // in kPa
            _sensors.humidity, // in %
            _sensors.voltage[0] / 1000, // In Volts
            _sensors.power[0] / 1000, // in W
            _sensors.power[1] / 1000, // in W
            _sensors.power[2] / 1000, // in W
            state,
            cameraOn,
            frameRate
            
        );

        // Send output
        printAllPorts(output);

        return true;
    }

    void writeConfig() {
        if (systemOkay) {
            cfg.writeConfig();
        }
    }

    void readConfig() {
        if (systemOkay)
            cfg.readConfig();
    }

    void checkInput() {
        if (DEBUGPORT.available() > 0) {
            readInput(&DEBUGPORT);
        }
        if (UI1.available() > 0) {
            readInput(&UI1);
        }
    }

    void printAllPorts(const char output[]) {
        UI1.println(output);
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
        if (pendingPowerOn || !cameraOn) {
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
            if (cameraOn && !pendingPowerOff) {
                printAllPorts("Shuting down camera...");
                sendShutdown();
            }
        }

        if (latestHum > cfg.getInt(HUMLIMIT)) {
            char output[64];
            sprintf(output,"Humidity %0.2f %% exceeds limit of %0.2f %%", latestHum, (float)cfg.getInt(HUMLIMIT));
            printAllPorts(output);
            badEnv = true;
            if (cameraOn && !pendingPowerOff) {
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

                printAllPorts("Going to sleep...");
                _zerortc.setAlarmTime(0, 0, 0);
                if (cfg.getInt(CHECKHOURLY) == 1) {
                    _zerortc.enableAlarm(RTCZero::MATCH_MMSS);
                }
                else {
                    _zerortc.enableAlarm(RTCZero::MATCH_SS);
                }
                if (cfg.getInt(STANDBY) == 1) {
                    DEBUGPORT.println("Would go into stanby here but currently disabled.");
                    //_zerortc.standbyMode();
                }
            }
        }
    }

    void sendShutdown() {
        if (cameraOn) {
            DEBUGPORT.println("Sending to Jetson: sudo shutdown -h now");
            JETSONPORT.println("sudo shutdown -h now\n");
            pendingPowerOff = true;
            pendingPowerOffTimer = _zerortc.getEpoch();
        }
        else {
            DEBUGPORT.println("Camera not powered on, not sending shutdown command");
        }
    }

    void strobeLED(int cameraTrig, int ledTrig) {
        int flashDuration = 0;
        if (ledTrig == LED_TRIG1) {
            flashDuration = cfg.getInt(FLASH1);
        }
        if (ledTrig == LED_TRIG2) {
            flashDuration = cfg.getInt(FLASH2);
        }
        if (ledTrig == LED_TRIG3) {
            flashDuration = cfg.getInt(FLASH3);
        }
        if (ledTrig == LED_TRIG4) {
            flashDuration = cfg.getInt(FLASH4);
        }
        
        int trigWidth = cfg.getInt(TRIGWIDTH);

        //Trig camera and delay
        digitalWrite(cameraTrig,HIGH);
        if (trigWidth < 10000) {
            delayMicroseconds(trigWidth/2);
        }
        else {
            delay(trigWidth/1000/2);
        }
        
        // Trig strobe and delay
        digitalWrite(ledTrig,HIGH);
        if (flashDuration < 10000) {
            delayMicroseconds(flashDuration);
        }
        else {
            delay(flashDuration/1000);
        }
        digitalWrite(ledTrig,LOW);
        
        // Lower camera line
        if (trigWidth < 10000) {
            delayMicroseconds(trigWidth/2);
        }
        else {
            delay(trigWidth/1000/2);
        }
        digitalWrite(cameraTrig,LOW);
    }

    void strobeAllLEDS() {
        int frameRate = cfg.getInt(FRAMERATE);
        int frameDelay = 1000 / frameRate; // in ms
        strobeLED(CAM_TRIG, LED_TRIG1);
        delay(frameDelay);
        strobeLED(CAM_TRIG, LED_TRIG2);
        delay(frameDelay);
        strobeLED(CAM_TRIG, LED_TRIG3);
        delay(frameDelay);
        strobeLED(CAM_TRIG, LED_TRIG4);
    }

    void strobe1() {
        strobeLED(CAM_TRIG, LED_TRIG1);
    }

    void strobe2() {
        strobeLED(CAM_TRIG, LED_TRIG2);
    }

    void strobe3() {
        strobeLED(CAM_TRIG, LED_TRIG3);
    }

    void strobe4() {
        strobeLED(CAM_TRIG, LED_TRIG4);
    }
        
};

#endif
