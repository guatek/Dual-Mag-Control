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
    delayMicroseconds(sys.highMagStrobeDuration);
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
    delayMicroseconds(sys.lowMagStrobeDuration);
    digitalWrite(LOW_MAG_STROBE_TRIG,LOW);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(LOW_MAG_CAM_TRIG,LOW);
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
    sys.cfg.addParam("POLLFREQ", "Rate of polling instruments", "Hz", 1, 50, 10);
    sys.cfg.addParam("DEPTHCHECKINTERVAL", "Time in seconds between depth checks for testing ascent/descent", "s", 10, 300, 30);
    sys.cfg.addParam("DEPTHTHRESHOLD", "Depth change threshold to denote ascent or descent", "mm", 500, 10000, 1000);
    sys.cfg.addParam("LOCALECHO", "When > 0, echo serial input", "", 0, 1, 1);
    sys.cfg.addParam("CMDTIMEOUT", "time in ms before timeout waiting for user input", "ms", 1000, 100000, 10000);
    sys.cfg.addParam("HWPORT0BAUD", "Serial Port 0 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("HWPORT1BAUD", "Serial Port 1 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("HWPORT2BAUD", "Serial Port 2 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("HWPORT3BAUD", "Serial Port 3 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam("STROBEDELAY", "Time between camera trigger and strobe trigger in us", "us", 5, 1000, 50);
    sys.cfg.addParam("FRAMERATE", "Camera frame rate in Hz", "Hz", 1, 30, 10);
    sys.cfg.addParam("TRIGWIDTH", "Width of the camera trigger pulse in us", "us", 30, 10000, 100);
    sys.cfg.addParam("LOWMAGCOLORFLASH", "Width of the low-mag white flash in us", "us", 1, 100000, 10);
    sys.cfg.addParam("LOWMAGREDFLASH", "Width of the low-mag far red flash in us", "us", 1, 100000, 10);
    sys.cfg.addParam("HIGHMAGCOLORFLASH", "Width of the high-mag white flash in us", "us", 1, 100000, 10);
    sys.cfg.addParam("HIGHMAGREDFLASH", "Width of the high-mag far red flash in us", "us", 1, 100000, 10);
    sys.cfg.addParam("FLASHTYPE", "0 = white strobes, 1 = far red strobes","", 0, 1, 0);
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

    sys.cfg.printConfig(&UI1);

    // Setup triggers and polling
    sys.setupTimers();
    
}

void loop() {

    sys.checkInput();
    sys.update();
    sys.checkVoltage();

    // wait for log interval while polling other sensors
    int logInt = sys.cfg.getInt("LOGINT");

    delay(logInt);
    Blink(10, 1);

}
