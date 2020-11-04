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