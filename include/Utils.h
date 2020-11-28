#ifndef _UTILS

#define _UTILS

#define PORT_BREAK_CHAR 5

#include <Arduino.h>

void Blink(int DELAY_MS, byte loops)
{
    pinMode(LED_BUILTIN, OUTPUT);
    while (loops--)
    {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(DELAY_MS);
        digitalWrite(LED_BUILTIN,LOW);
        delay(DELAY_MS);  
    }
}

void portpass(Stream * in, Stream * out, bool localecho = false) {
    while (true) {
        if (in->available()) {
            unsigned char c = in->read();
            if (c == PORT_BREAK_CHAR) {
                break;
            }
            else{
                out->write(c);
            }
        }
        if (out->available()) {
            in->write(out->read());
        }
    }
}

bool confirm(Stream * in, const char * prompt, unsigned int cmdTimeout) {
    unsigned long startTimer = millis();
    in->println();
    in->print(prompt);

    while (startTimer <= millis() && millis() - startTimer < cmdTimeout) {

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



#endif