#ifndef _SYSTEMTRIGGER

#define _SYSTEMTRIGGER

#include <Adafruit_ZeroTimer.h>

#define HIGH_MAG_CAM_TRIG SWIO
#define LOW_MAG_CAM_TRIG SWCLK
#define HIGH_MAG_STROBE_TRIG A1
#define LOW_MAG_STROBE_TRIG 4

// timer tester
Adafruit_ZeroTimer highMagTimer = Adafruit_ZeroTimer(3);
Adafruit_ZeroTimer lowMagTimer = Adafruit_ZeroTimer(5);

//define the interrupt handlers
void TC3_Handler(){
  Adafruit_ZeroTimer::timerHandler(3);
}

//define the interrupt handlers
void TC5_Handler(){
  Adafruit_ZeroTimer::timerHandler(5);
}

// the timer 3 callbacks
void HighMagCallback()
{
  digitalWrite(HIGH_MAG_CAM_TRIG,HIGH);
  delayMicroseconds(50);
  digitalWrite(HIGH_MAG_STROBE_TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(HIGH_MAG_STROBE_TRIG,LOW);
  delayMicroseconds(50);
  digitalWrite(HIGH_MAG_CAM_TRIG,LOW);
}

// the timer 5 callbacks
void LowMagCallback()
{
  digitalWrite(LOW_MAG_CAM_TRIG,HIGH);
  delayMicroseconds(50);
  digitalWrite(LOW_MAG_STROBE_TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(LOW_MAG_STROBE_TRIG,LOW);
  delayMicroseconds(50);
  digitalWrite(LOW_MAG_CAM_TRIG,LOW);
}

void configTimer(float freq, uint8_t * divider, uint16_t * compare, tc_clock_prescaler * prescaler) {
       // Set up the flexible divider/compare
    //uint8_t divider  = 1;
    //uint16_t compare = 0;
    *prescaler = TC_CLOCK_PRESCALER_DIV1;

    if ((freq < 24000000) && (freq > 800)) {
        *divider = 1;
        *prescaler = TC_CLOCK_PRESCALER_DIV1;
        *compare = 48000000/freq;
    } else if (freq > 400) {
        *divider = 2;
        *prescaler = TC_CLOCK_PRESCALER_DIV2;
        *compare = (48000000/2)/freq;
    } else if (freq > 200) {
        *divider = 4;
        *prescaler = TC_CLOCK_PRESCALER_DIV4;
        *compare = (48000000/4)/freq;
    } else if (freq > 100) {
        *divider = 8;
        *prescaler = TC_CLOCK_PRESCALER_DIV8;
        *compare = (48000000/8)/freq;
    } else if (freq > 50) {
        *divider = 16;
        *prescaler = TC_CLOCK_PRESCALER_DIV16;
        *compare = (48000000/16)/freq;
    } else if (freq > 12) {
        *divider = 64;
        *prescaler = TC_CLOCK_PRESCALER_DIV64;
        *compare = (48000000/64)/freq;
    } else if (freq > 3) {
        *divider = 256;
        *prescaler = TC_CLOCK_PRESCALER_DIV256;
        *compare = (48000000/256)/freq;
    } else if (freq >= 0.75) {
        *divider = 1024;
        *prescaler = TC_CLOCK_PRESCALER_DIV1024;
        *compare = (48000000/1024)/freq;
    } else {
        Serial.println("Invalid frequency");
    }
    Serial.print("Divider:"); Serial.println(*divider);
    Serial.print("Compare:"); Serial.println(*compare);
    Serial.print("Final freq:"); Serial.println((int)(48000000/(*compare)));
}

void configTriggers(float freq) {

    pinMode(HIGH_MAG_CAM_TRIG,OUTPUT);
    pinMode(LOW_MAG_CAM_TRIG,OUTPUT);
    pinMode(HIGH_MAG_STROBE_TRIG,OUTPUT);
    pinMode(LOW_MAG_STROBE_TRIG,OUTPUT);

    Serial.println("Trigger Configuration");

    Serial.print("Desired freq (Hz):");
    Serial.println(freq);

    uint8_t divider  = 1;
    uint16_t compare = 0;
    tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

    configTimer(freq, &divider, &compare, &prescaler);

    highMagTimer.enable(false);
    highMagTimer.configure(prescaler,       // prescaler
            TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
            TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
            );

    highMagTimer.setCompare(0, compare);
    highMagTimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, HighMagCallback);
    highMagTimer.enable(true);

    configTimer(freq, &divider, &compare, &prescaler);

    lowMagTimer.enable(false);
    lowMagTimer.configure(prescaler,       // prescaler
            TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
            TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
            );

    lowMagTimer.setCompare(0, compare);
    lowMagTimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, LowMagCallback);
    lowMagTimer.enable(true);


}

#endif



