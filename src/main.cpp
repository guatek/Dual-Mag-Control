#include "Config.h"
#include "SystemControl.h"
#include "Sensors.h"
#include "SystemTrigger.h"
#include "SystemConfig.h"
#include "Utils.h"

// Global system control variable
SystemControl sys;

// High Mag Trigger Callback
void HighMagCallback()
{
    digitalWrite(HIGH_MAG_CAM_TRIG,HIGH);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(HIGH_MAG_STROBE_TRIG,HIGH);
    if (sys.highMagStrobeDuration-FLASH_DELAY_OFFSET > MIN_FLASH_DURATION)
        delayMicroseconds(sys.highMagStrobeDuration-FLASH_DELAY_OFFSET);
    digitalWrite(HIGH_MAG_STROBE_TRIG,LOW);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(HIGH_MAG_CAM_TRIG,LOW);
}

// Low Mag Trigger Callback
void LowMagCallback()
{
    digitalWrite(LOW_MAG_CAM_TRIG,HIGH);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(LOW_MAG_STROBE_TRIG,HIGH);
    if (sys.lowMagStrobeDuration-FLASH_DELAY_OFFSET > MIN_FLASH_DURATION)
    delayMicroseconds(sys.lowMagStrobeDuration-FLASH_DELAY_OFFSET);
    digitalWrite(LOW_MAG_STROBE_TRIG,LOW);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(LOW_MAG_CAM_TRIG,LOW);
}

// Wrapper for updaing timers and flashes from callback functions
void setTriggers() {
    sys.setTriggers();
}

void setFlashes() {
    sys.configureFlashDurations();
}

void setPolling() {
    sys.setPolling();
}

// wrapper for turning system on
void turnOnCamera() {
    if (sys.cfg.getInt("PROFILEMODE") == 1) {
        sys.turnOnCamera();
    }
}



void setup() {

    // Start the debug port
    DEBUGPORT.begin(115200);

    delay(4000);

    // Wait until serial port is opened
    //while (!Serial) { delay(10); }
    //while (!Serial0) { delay(10); }

    // Startup all system processes
    sys.begin();

    // Add config parameters for system
    sys.cfg.addParam("LOGINT", "Time in ms between log events", "ms", 0, 100000, 250);
    sys.cfg.addParam("POLLFREQ", "Rate of polling instruments", "Hz", 1, 50, 10, false, setPolling);
    sys.cfg.addParam("DEPTHCHECKINTERVAL", "Time in seconds between depth checks for testing ascent/descent", "s", 10, 300, 30);
    sys.cfg.addParam("DEPTHTHRESHOLD", "Depth change threshold to denote ascent or descent", "mm", 500, 10000, 1000);
    sys.cfg.addParam("LOCALECHO", "When > 0, echo serial input", "", 0, 1, 1);
    sys.cfg.addParam("CMDTIMEOUT", "time in ms before timeout waiting for user input", "ms", 1000, 100000, 10000);
    sys.cfg.addParam("HWPORT0BAUD", "Serial Port 0 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("HWPORT1BAUD", "Serial Port 1 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("HWPORT2BAUD", "Serial Port 2 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("HWPORT3BAUD", "Serial Port 3 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("STROBEDELAY", "Time between camera trigger and strobe trigger in us", "us", 5, 1000, 50, false, setFlashes);
    sys.cfg.addParam("FRAMERATE", "Camera frame rate in Hz", "Hz", 1, 30, 10, false, setTriggers);
    sys.cfg.addParam("TRIGWIDTH", "Width of the camera trigger pulse in us", "us", 30, 10000, 100, false, setFlashes);
    sys.cfg.addParam("LOWMAGCOLORFLASH", "Width of the low-mag white flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam("LOWMAGREDFLASH", "Width of the low-mag far red flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam("HIGHMAGCOLORFLASH", "Width of the high-mag white flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam("HIGHMAGREDFLASH", "Width of the high-mag far red flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam("FLASHTYPE", "0 = white strobes, 1 = far red strobes","", 0, 1, 0, false, setFlashes);
    sys.cfg.addParam("PROFILEMODE","0 = upcast only, 1 = always on", "", 0, 1, 0);

    sys.cfg.addParam("LOWVOLTAGE", "Voltage in mV where we shut down system", "mV", 10000, 14000, 11500);
    sys.cfg.addParam("CHECKHOURLY", "0 = check every minute, 1 = check every hour", "", 0, 1, 0);

    // Start the remaining serial ports
    HWPORT0.begin(sys.cfg.getInt("HWPORT0BAUD"));
    HWPORT1.begin(sys.cfg.getInt("HWPORT1BAUD"));
    HWPORT2.begin(sys.cfg.getInt("HWPORT2BAUD"));
    HWPORT3.begin(sys.cfg.getInt("HWPORT3BAUD"));

    // Config the SERCOM muxes AFTER starting the ports
    configSerialPins();

    // Load the last config from EEPROM
    sys.readConfig();

    sys.cfg.printConfig(&DEBUGPORT);
    sys.cfg.printConfig(&UI1);

    // Setup flashes triggers and polling
    setFlashes();
    setTriggers();
    setPolling();

    // If camera is set to be always on, power it up now
    if (sys.cfg.getInt("PROFILEMODE") == 1) {
        sys.turnOnCamera();
    }
    
}

void loop() {

    sys.checkInput();
    sys.update();
    sys.checkVoltage();

    int logInt = sys.cfg.getInt("LOGINT");

    delay(logInt);
    Blink(10, 1);

}
