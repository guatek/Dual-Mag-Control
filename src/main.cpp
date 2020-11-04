#include <SPIFlash.h>    //get it here: https://github.com/LowPowerLab/SPIFlash
#include "SystemControl.h"
#include "Sensors.h"
#include "Utils.h"

#define SEALEVELPRESSURE_HPA (1013.25)

SystemControl sys((Stream*)&Serial);
Sensors sensors((Stream*)&Serial);

void setup() {
    Serial.begin(115200);
    // Wait until serial port is opened
    while (!Serial) { delay(10); }

    bool status;

    status = sensors.begin();
    
}

void loop() {

    sensors.printEnv();
    sensors.printPower();
    sys.checkInput();

    delay(500);

}
