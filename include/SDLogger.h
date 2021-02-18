#ifndef _SDLOGGER

#define _SDLOGGER

#include "RTCZero.h"
#include "Config.h"

// A simple read/write example for SD.h.
// Mostly from the SD.h ReadWrite example.
//
// Your SD must be formatted FAT16/FAT32.
//
// Set USE_SD_H nonzero to use SD.h.
// Set USE_SD_H zero to use SdFat.h.
//
#define USE_SD_H 0
//
#if USE_SD_H
#include <SD.h>
#else  // USE_SD_H
#include "SdFat.h"
SdFat _SD;
#endif  // USE_SD_H

// Modify SD_CS_PIN for your board.
#define SD_CS_PIN 13


class SDLogger {

    public:
    SDLogger(unsigned int maxLinesPerFile=10000) {

        this->maxLinesPerFile=maxLinesPerFile;
        linesWritten = 0;
        detected = false;
        initialized = false;

    }

    bool SDCardPresent() {
        detected = digitalRead(SDCARD_DETECT);
        return detected;
    }

    bool SdCardInit() {
        initialized = !_SD.begin(SD_CS_PIN);
    }

    bool createNewFile(RTCZero * rtc) {
        if (initialized) {
            
            // Create the directory path in the format: /YYYY/MM/DD
            char dirPath[16];
            sprintf(dirPath,"/%04d/%02d/%02d", rtc->getYear(), rtc->getMonth(), rtc->getDay());
            
            // If it does not exist, create it and chdir to it
            if (!_SD.exists(dirPath)) {
                // Create the dir and chdir to it if succesfull
                if (_SD.mkdir(dirPath, true)) {
                    _SD.chdir(dirPath);
                }
            }

            // Check again that dir exists before opening file
            if (_SD.exists(dirPath)) {
                char fileName[16];
                sprintf(fileName,"%02d%02d%02d.log", rtc->getHours(), rtc->getMinutes(), rtc->getSeconds());
                myFile = _SD.open(fileName, FILE_WRITE);
                linesWritten = 0;
                return true;
            }
        }

        return false;
    }

    bool writeString(RTCZero * rtc, const char * data) {
        if (initialized) {

            // Create a new file/dir if needed
            if (!myFile || linesWritten >= maxLinesPerFile) {
                if (myFile) {
                    myFile.close(); // close open file if needed
                }
                createNewFile(rtc);
            }

            // If file is valid write to it
            if (myFile) {
                myFile.write(data, strlen(data));
                linesWritten++;
                return true;
            }
        }

        return false;
    }

    private:
    bool detected;
    bool initialized;
    unsigned int linesWritten;
    unsigned int maxLinesPerFile;
    File myFile;

};

#endif