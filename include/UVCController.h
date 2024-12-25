#ifndef _UVCCONTROLLER

#define _UVCCONTROLLER

#include <Adafruit_ZeroTimer.h>

#define DUTYCYCLE 10

// Flash Triggers
Adafruit_ZeroTimer _uvcPWMTimer = Adafruit_ZeroTimer(2);

class UVCController {

    public:
    float dutyCycle;
    float freq;
    bool enabled;
    int pin;

    UVCController(float dutyCycle = 10, float freq = 100, int pin) {
        this->dutyCycle = dutyCycle;
        this->freq = freq;
        this->enabled = false;
        this->pin = pin;

    }

    bool setFreq(float freq) {
        this->freq = freq;
        return true;
    }

    bool setDutyCycle(float dutyCycle) {
        this->dutyCycle = dutyCycle;
        return true;
    }

    bool setPin(int pin) {
        this->pin = pin;
    }

    bool enable() {
        _uvcPWMTimer.configure(TC_CLOCK_PRESCALER_DIV1, // prescaler
                        TC_COUNTER_SIZE_16BIT,   // bit width of timer/counter
                        TC_WAVE_GENERATION_NORMAL_PWM // frequency or PWM mode
                        );
        if (! _uvcPWMTimer.PWMout(true, 0, this->pin)) {
            Serial.println("Failed to configure PWM output");
        }

        _uvcPWMTimer.setCompare(0, 0xFFFF/4);
        _uvcPWMTimer.enable(true);
    }

    bool disable() {
        _uvcPWMTimer.enable(false);
    }



}


#endif



