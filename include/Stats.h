#ifndef _STATS

#define _STATS

#include "Arduino.h"
#include "config.h"

#define MAX_BUFFER_SIZE 128

template <class T>
class MovingAverage {

    private:
    T buffer[MAX_BUFFER_SIZE];
    int index;
    int samples;

    public:

    MovingAverage(int samples=64) {
        this->samples = samples;
        if (this->samples >= MAX_BUFFER_SIZE) {
            DEBUGPORT.println("Samples exceed max buffer size, setting to max buffer size.");
            this->samples = MAX_BUFFER_SIZE;
        }
        this->index = 0;
    }

    float update(T newSample) {
        if (index < samples) {
            buffer[index++] = newSample;
        }
        else {
            for (int i = 1; i < index; i++) {
                buffer[i-1] = buffer[i];
            }
            buffer[samples-1] = newSample;
        }
        if (index == 0) {
            return newSample;
        }
        else {
            float avg = 0.0;
            for (int i = 0; i < index; i++) {
                avg += buffer[i];
            }
            avg /= index;
            return avg;
        } 
    }

    void clear() {
        index -= 0;
    }

};

#endif