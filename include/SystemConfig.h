#ifndef _SYSTEMCONFIG

#define _SYSTEMCONFIG

#include <Arduino.h>
#include "Config.h"

#define MAX_PARAMS 256

//////////////////////////////////////////
// flash(SPI_CS, MANUFACTURER_ID)
// SPI_CS          - CS pin attached to SPI flash chip (8 in case of Moteino)
// MANUFACTURER_ID - OPTIONAL, 0x1F44 for adesto(ex atmel) 4mbit flash
//                             0xEF30 for windbond 4mbit flash
//                             0xEF40 for windbond 64mbit flash
//////////////////////////////////////////
uint16_t _expectedDeviceID=0xEF30;
SPIFlash _flash(SS_FLASHMEM, _expectedDeviceID);

template <class T>
class ConfigParam {
    public:
        const char * desc;
        const char * name;
        const char * units;
        void (*callback)(); 
        bool isFloat;
        uint32_t uid;
        T minVal;
        T maxVal;
        T val;
        
        ConfigParam(const char * name, const char * desc, const char * units, int uid, T minVal, T maxVal, T defaultVal, bool isFloat = false, void (*callback)() = NULL) {
            this->name = name;
            this->desc = desc;
            this->units = units;
            this->uid = uid;
            this->isFloat = isFloat;
            this->minVal = minVal;
            this->maxVal = maxVal;
            this->val = defaultVal;
            this->callback = callback;
        }

        void writeToFlash() {
            _flash.writeBytes(uid, (void*)&val, (uint16_t)sizeof(T));
            DEBUGPORT.print("Write value [@");
            DEBUGPORT.print(uid);
            DEBUGPORT.print("]: ");
            DEBUGPORT.println(val);
        }

        void readFromFlash() {
            T newVal;
            _flash.readBytes(uid, (void*)&newVal, (uint16_t)sizeof(T));
            setVal(newVal);
            DEBUGPORT.print("Read value [@");
            DEBUGPORT.print(uid);
            DEBUGPORT.print("]: ");
            DEBUGPORT.println(val);
        }

        bool readFromCLI(Stream * in, T * val, char exitChar, unsigned int cmdTimeout) {
            unsigned long startTimer = millis();
            in->println();
            in->print("Enter a value for ");
            in->print(name);
            in->print(" [");
            in->print(minVal);
            in->print(",");
            in->print(maxVal);
            in->print("]: ");

            int bufferLength = 64;
            char buffer[bufferLength];
            int bufferIndex = 0;

            while (startTimer <= millis() && millis() - startTimer < cmdTimeout) {

                // Wait on user input
                if (in->available()) {
                    // Read the next char and reset timer          
                    char c = in->read();
                    if (c == exitChar) {
                        return false;
                    }
                    else if (c == '\n') {
                        if (bufferIndex < bufferLength) {
                            buffer[bufferIndex] = '\0';
                            int result = 0;
                            T newVal;
                            if (isFloat) {
                                result = sscanf(buffer, "%f", (float*)&newVal);
                            }
                            else {
                                result = sscanf(buffer, "%d", (int*)&newVal);
                            }
                            if (result == 1 && (minVal <= newVal && newVal <= maxVal)) {
                                *val = newVal;
                                return true;
                            }
                            else {
                                in->println("Invalid entry.");
                                bufferIndex = 0;
                            }
                        }
                    }
                    else {
                        if (bufferIndex < bufferLength) {
                            buffer[bufferIndex++] = c;
                        }
                    }
                }
            }

            return false;
        }

        bool checkVal(T newVal) {
            if (newVal >= minVal && newVal <= maxVal) { 
                return true;
            }
            else {
                return false;
            }
        }

        bool setVal(T newVal) {
            if (newVal >= minVal && newVal <= maxVal) {
                val = newVal;
                // Call function if provided
                if (callback != NULL) {
                    callback();
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        bool setValFromString(char * input, int len) {
            T newVal;
            int result;
            if (isFloat) {
                result = sscanf(input, "%f", (float*)&newVal);
            }
            else {
                result = sscanf(input, "%d", (int*)&newVal);
            }
            if (result == 1 && newVal >= minVal && newVal <= maxVal) {
                val = newVal;
                // Call function if provided
                if (callback != NULL) {
                    DEBUGPORT.println("\ncallback fired");
                    callback();
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        void print(Stream * ui) {
            ui->print(name);
            ui->print(" [ ");
            ui->print(minVal);
            ui->print(",");
            ui->print(val);
            ui->print(",");
            ui->print(maxVal);
            ui->print(" ] : ");
            ui->println(desc);
        }
};

// Class to hold all of the system config and faciliate updating the config over serial port
// 
// IMPORTANT: To the extent possible try to always use ints for variables
class SystemConfig {
    public:
        ConfigParam<int> * intParams[MAX_PARAMS];
        ConfigParam<float> * floatParams[MAX_PARAMS];
        int nIntParams;
        int uid;
        int nFloatParams;   

        SystemConfig() {
            nIntParams = 0;
            nFloatParams = 0;
            uid = 0;
        }

        template <class T>
        bool addParam(const char * name, const char * desc, const char * units, T minVal, T maxVal, T defaultVal, bool isFloat = false, void (*callback)() = NULL) {
            if (!isFloat && nIntParams < MAX_PARAMS) {
                intParams[nIntParams] = new ConfigParam <int> (name, desc, units, uid, minVal, maxVal, defaultVal, isFloat, callback);
                nIntParams += 1;
                uid += sizeof(int);
                return true;
            }
            else if (nFloatParams < MAX_PARAMS) {
                floatParams[nFloatParams] = new ConfigParam <float> (name, desc, units, nIntParams, minVal, maxVal, defaultVal, isFloat, callback);
                nFloatParams += 1;
                uid += sizeof(float);
                return true;
            }
            else {
                return false;
            }
        }

        void printConfig(Stream * ui) {
            for (int i=0; i < nIntParams; i++){
                intParams[i]->print(ui);
            }
            for (int i=0; i < nFloatParams; i++){
                floatParams[i]->print(ui);
            }
        }

        int getInt(const char * name) {
            // Check int params
            for (int i = 0; i < nIntParams; i++) {
                if (strncmp(intParams[i]->name, name, strlen(name)) == 0) {
                    return intParams[i]->val;
                }
            }
            return 0;
        }

        float getFloat(const char * name) {
            // check float params
            for (int i = 0; i < nFloatParams; i++) {
                if (strncmp(floatParams[i]->name, name, strlen(name)) == 0) {
                    return floatParams[i]->val;
                }
            }
            return 0.0;
        }

        template <class T>
        bool set(const char * name, T newVal) {
            // Check int params
            for (int i = 0; i < nIntParams; i++) {
                if (strncmp(intParams[i]->name, name, strlen(name)) == 0) {
                    return intParams[i]->setVal(newVal);
                }
            }

            // check float params
            for (int i = 0; i < nFloatParams; i++) {
                if (strncmp(floatParams[i]->name, name, strlen(name)) == 0) {
                    return floatParams[i]->setVal(newVal);
                }
            }
        }

        bool readIntFromUI(const char * name, Stream * in, int * val, char exitChar, int cmdTimeout) {
            // Check int params
            for (int i = 0; i < nIntParams; i++) {
                if (strncmp(intParams[i]->name, name, strlen(name)) == 0) {
                    return intParams[i]->readFromCLI(in, val, exitChar, cmdTimeout);
                }
            }
        }

        bool parseConfigCommand(char * cmd, Stream * ui) {
            if (cmd == NULL) {
                printConfig(ui);
                return true;
            }
            else {
                // Try to parse and set config
                char * name = strtok(cmd, ",");
                if (name == NULL)
                    return false;
                char * val = strtok(NULL, ",");
                if (val == NULL)
                    return false;
                
                // Check int params
                for (int i = 0; i < nIntParams; i++) {
                    if (strncmp(intParams[i]->name, name, strlen(name)) == 0) {
                        bool updated = intParams[i]->setValFromString(val, strlen(val));
                        if (updated) {
                            ui->print("\r\nUpdated : ");
                            intParams[i]->print(ui);
                        }
                        else {
                            ui->println("Invalid entry.");
                        }
                        return updated;
                    }
                }

                // check float params
                for (int i = 0; i < nFloatParams; i++) {
                    if (strncmp(floatParams[i]->name, name, strlen(name)) == 0) {
                        bool updated = floatParams[i]->setValFromString(val, strlen(val));
                        if (updated) {
                            ui->print("\r\nUpdated : ");
                            floatParams[i]->print(ui);
                        }
                        else {
                            ui->println("Invalid entry.");
                        }
                        return updated;
                    }
                }
                
            }

            // If we didn't find any params to set return false
            return false;

        }

        void writeConfig() {

            // erase block first so we can write to it
            _flash.blockErase4K(0);

            for (int i = 0; i < nIntParams; i++) {
                intParams[i]->writeToFlash();
            }
            for (int i = 0; i < nFloatParams; i++) {
                floatParams[i]->writeToFlash();
            }
        }

        void readConfig() {
            for (int i = 0; i < nIntParams; i++) {
                intParams[i]->readFromFlash();
            }
            for (int i = 0; i < nFloatParams; i++) {
                floatParams[i]->readFromFlash();
            }
        }
};

#endif