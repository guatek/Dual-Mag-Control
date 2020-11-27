#ifndef _SYSTEMCONTROL

#define _SYSTEMCONTROL

#include <Arduino.h>
#include <RTCZero.h>
#include <RTCLib.h>
#include "Config.h"
#include "DeepSleep.h"
#include "SPIFlash.h"
#include "Sensors.h"
#include "Stats.h"
#include "SystemConfig.h"
#include "SystemTrigger.h"
#include "RBRInstrument.h"
#include "Utils.h"

#define CMD_CHAR '!'
#define PROMPT "DM > "
#define CMD_BUFFER_SIZE 128

#define STROBE_POWER 7
#define CAMERA_POWER 6

// Global Sensors
Sensors _sensors;

// Global RTCZero
RTCZero _zerortc;

//Global RTCLib
RTC_DS3231 _ds3231;

// RBR instrument
RBRInstrument _rbr;

// Static polling function for instruments
void pollInstruments() {
    _rbr.readData(&RBRPORT);
}

class SystemControl
{
    private:
    float lastDepth;
    bool systemOkay;
    bool ds3231Okay;
    bool cameraOn;
    char cmdBuffer[CMD_BUFFER_SIZE];
    bool rbrData;
    int state;
    unsigned long timestamp;
    unsigned long lastDepthCheck;
    MovingAverage<float> avgVoltage;

    bool confirm(Stream * in, const char * prompt) {
        unsigned long startTimer = millis();
        in->println();
        in->print(prompt);

        while (startTimer <= millis() && millis() - startTimer < (unsigned int)cfg.getInt("CMDTIMEOUT")) {

            // Wait on user input
            if (in->available()) {
                // Read the next char and reset timer          
                char c = in->read();
                if (c == 'Y' || c == 'y') {
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        return false;
    }

    int checkCommand(char * input, const char * command, int n) {
        size_t maxLength = n;
        
        // Coerce to shortest string
        if (strlen(input) < maxLength)
            maxLength = strlen(input);
        if (strlen(command) < maxLength)
            maxLength = strlen(command);

        for (unsigned int i=0; i < maxLength; i++) {
            if (tolower(input[i]) < tolower(command[i]))
                return -1;
            if (tolower(input[i]) > tolower(command[i]))
                return 1;
        }

        // string match up to n chars
        return 0;
    }
    
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
                        if (cmd != NULL && checkCommand(cmd,"CFG", 3) == 0) {
                            cfg.parseConfigCommand(rest, in);
                        }

                        // PORTPASS (pass through to other serial ports)
                        if (cmd != NULL && checkCommand(cmd,"PORTPASS", 8) == 0) {
                            doPortPass(in, rest);
                        }

                        // SETTIME (set time from string)
                        if (cmd != NULL && checkCommand(cmd,"SETTIME", 7) == 0) {
                            setTime(rest, in);
                        }

                        // WRITECONFIG (save the current config to EEPROM)
                        if (cmd != NULL && checkCommand(cmd,"WRITECONFIG", 11) == 0) {
                            writeConfig();
                        }

                        // READCONFIG (read the current config to EEPROM)
                        if (cmd != NULL && checkCommand(cmd,"READCONFIG", 10) == 0) {
                            readConfig();
                        }

                        if (cmd != NULL && checkCommand(cmd,"CAMERAON",8) == 0) {
                            if (confirm(in, "Are you sure you want to power ON camera ? [y/N]: "))
                                turnOnCamera();
                        }

                        if (cmd != NULL && checkCommand(cmd,"CAMERAOFF",9) == 0) {
                            if (confirm(in, "Are you sure you want to power OFF camera ? [y/N]: "))
                                turnOffCamera();
                        }

                        if (cmd != NULL && checkCommand(cmd,"SHUTDOWNJETSON",14) == 0) {
                            if (confirm(in, "Are you sure you want to shutdown jetson ? [y/N]: "))
                                sendShutdown();
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
                        if (index < 0)
                            index = 0;
                        if (cfg.getInt("LOCALECHO")) {
                           in->write("\b \b");
                        }
                    }
                    else {
                        cmdBuffer[index++] = c;
                        if (cfg.getInt("LOCALECHO"))
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
                    portpass(in, &HWPORT0, cfg.getInt("LOCALECHO") == 1);
                    break;
                case '1':
                    portpass(in, &HWPORT1, cfg.getInt("LOCALECHO") == 1);
                    break;
                case '2':
                    portpass(in, &HWPORT2, cfg.getInt("LOCALECHO") == 1);
                    break;
                case '3':
                    portpass(in, &HWPORT3, cfg.getInt("LOCALECHO") == 1);
                    break;
            }
        }
    }

    void setTime(char * timeString, Stream * ui) {
        if (timeString != NULL) {
            // if we have ds3231 set that first
            DateTime dt(timeString);
            if (ds3231Okay) {
                _ds3231.adjust(dt.unixtime());
            }
            _zerortc.setEpoch(dt.unixtime());
        }
    }
                      
 
    public:

    SystemConfig cfg;
    int trigWidth;
    int lowMagStrobeDuration;
    int highMagStrobeDuration;
    int flashType;
  
    SystemControl() {
        systemOkay = false;
        rbrData = false;
        state = 0;
        timestamp = 0;
        ds3231Okay = false;
    }

    bool begin() {

        //Turn off strobe and camera power
        pinMode(CAMERA_POWER, OUTPUT);
        pinMode(STROBE_POWER, OUTPUT);

        digitalWrite(CAMERA_POWER, LOW);
        digitalWrite(STROBE_POWER, HIGH);

        // Start RTC
        _zerortc.begin();

        // Start DS3231
        ds3231Okay = true;
        if (!_ds3231.begin()) {
            DEBUGPORT.println("Could not init DS3231, time will be lost on power cycle.");
            ds3231Okay = false;
        }


        lastDepthCheck = _zerortc.getEpoch();
        lastDepth = 0.0;

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

    bool turnOnCamera() {
        cameraOn = true;
        digitalWrite(CAMERA_POWER, HIGH);
        digitalWrite(STROBE_POWER, LOW);

        return true;
    }

    bool turnOffCamera() {
        cameraOn = false;
        digitalWrite(CAMERA_POWER, LOW);
        digitalWrite(STROBE_POWER, HIGH);

        return true;
    }

    bool update() {

        // Run updates and check for new data
        _sensors.update();

        // Build log string and send to UIs
        char output[256];

        float c = -1.0;
        float t = -1.0;
        float d = -1.0;

        uint32_t unixtime; 

        char timeString[64];
        
        sprintf(timeString,"%s","YYYY-MM-DD hh:mm:ss");

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

        if (_rbr.haveNewData()) {
            c = _rbr.conductivity();
            t = _rbr.temperature();
            d = _rbr.pressure();
            _rbr.invalidateData();

            // Check state (float up, down ratchet)
            if (unixtime - lastDepthCheck > (unsigned int)cfg.getInt("DEPTHCHECKINTERVAL")) {
                float delta_depth = d - lastDepth;
                if ((state == -1 || state == 0) && delta_depth > ((float)cfg.getInt("DEPTHTHRESHOLD"))/1000) {
                    state = 1; // ascent to descent
                }
                if ((state == 1 || state == 0)  && delta_depth < ((float)cfg.getInt("DEPTHTHRESHOLD"))/1000) {
                    state = -1; // descent to ascent
                }
                lastDepthCheck = unixtime;
                lastDepth = d;
            }
        }


        // The system log string, note this requires enabling printf_float build
        // option work show any output for floating point values
        sprintf(output, "$DM,%s.%03u,%0.3f,%0.3f,%0.2f,%0.2f,%0.2f,%0.2f,%0.3f,%0.3f,%0.3f,%d",

            timeString,
            ((unsigned int) millis()) % 1000,
            _sensors.temperature, // In C
            _sensors.pressure / 1000, // in kPa
            _sensors.humidity, // in %
            _sensors.voltage[0] / 1000, // In Volts
            _sensors.power[0] / 1000, // in W
            _sensors.power[1] / 1000, // in W
            c,
            t,
            d,
            state
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
        if (UI2.available() > 0) {
            readInput(&UI2);
        }
    }

    void printAllPorts(const char output[]) {
        UI1.println(output);
        UI2.println(output);
        DEBUGPORT.println(output);
    }

    void checkVoltage() {

        // Update moving average of voltage
        //float latestVoltage = avgVoltage.update(_sensors.voltage[0]);
        float latestVoltage = _sensors.voltage[0];

        // If battery voltage is too low, notify and sleep
        // If the camera is running at this point, shut it down first
        if (latestVoltage < cfg.getInt("LOWVOLTAGE")) {
            char output[256];
            sprintf(output,"Voltage %f below threshold %d", latestVoltage, cfg.getInt("LOWVOLTAGE"));
            printAllPorts(output);
            if (cameraOn) {
                printAllPorts("Camera is ON! Sending Shutdown and waiting for 30 seconds...");
                sendShutdown();
                delay(30000);
                printAllPorts("Turning OFF camera power");
                turnOffCamera();
            }
            delay(2000);
            printAllPorts("Going to sleep...");
            _zerortc.setAlarmTime(0, 0, 0);
            if (cfg.getInt("CHECKHOURLY") == 1) {
                _zerortc.enableAlarm(RTCZero::MATCH_MMSS);
            }
            else {
                _zerortc.enableAlarm(RTCZero::MATCH_SS);
            }
            if (cfg.getInt("STANDBY") == 1) {
                DEBUGPORT.println("Would go into stanby here but currently disabled.");
                //_zerortc.standbyMode();
            }
        }
    }

    void sendShutdown() {
        if (cameraOn) {
            JETSONPORT.println("sudo shutdown -h now\n");
        }
        else {
            DEBUGPORT.println("Camera not powered on, not sending shutdown command");
        }
    }

    void configureFlashDurations() {
        // Set global delays for ISRs
        trigWidth = cfg.getInt("TRIGWIDTH");
        flashType = cfg.getInt("FLASHTYPE");
        if (flashType == 0) {
            digitalWrite(FLASH_TYPE_PIN,HIGH);
            lowMagStrobeDuration = cfg.getInt("LOWMAGCOLORFLASH");
            highMagStrobeDuration = cfg.getInt("HIGHMAGCOLORFLASH");
        }
        else {
            digitalWrite(FLASH_TYPE_PIN,LOW);
            lowMagStrobeDuration = cfg.getInt("LOWMAGREDFLASH");
            highMagStrobeDuration = cfg.getInt("HIGHMAGREDFLASH");
        }
    }

    void setTriggers() {
        configTriggers(cfg.getInt("FRAMERATE"));
    }

    void setPolling() {
        configPolling(cfg.getInt("POLLFREQ"), pollInstruments);
    }
        
};

#endif
