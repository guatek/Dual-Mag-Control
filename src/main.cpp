#include "Config.h"
#include "SystemControl.h"
#include "Sensors.h"
#include "SystemConfig.h"
#include "Utils.h"

// Global system control variable
SystemControl sys;

// wrapper for turning system on
void turnOnCamera() {
    if (sys.cfg.getInt("PROFILEMODE") == 1) {
        sys.turnOnCamera();
    }
}



void setup() {

    // Set initial state of control pins
    pinMode(CAMERA_POWER, OUTPUT);
    pinMode(LASER_ENABLE, OUTPUT);
    pinMode(LED_TRIG1, OUTPUT);
    pinMode(LED_TRIG2, OUTPUT);
    pinMode(LED_TRIG3, OUTPUT);
    pinMode(LED_TRIG4, OUTPUT);
    pinMode(TRIG_0_0, OUTPUT);
    pinMode(TRIG_1_0, OUTPUT);
    pinMode(TRIG_4_0, OUTPUT);

    digitalWrite(CAMERA_POWER, HIGH);
    digitalWrite(LED_TRIG1, LOW);
    digitalWrite(LED_TRIG2, LOW);
    digitalWrite(LED_TRIG3, LOW);
    digitalWrite(LED_TRIG4, LOW);
    digitalWrite(TRIG_0_0, LOW);
    digitalWrite(TRIG_1_0, LOW);
    digitalWrite(TRIG_4_0, LOW);
    digitalWrite(LASER_ENABLE, LOW);

    // Start the debug port
    DEBUGPORT.begin(115200);

    delay(4000);

    // Wait until serial port is opened
    //while (!Serial) { delay(10); }
    //while (!Serial0) { delay(10); }

    // Startup all system processes
    sys.begin();

    // Add config parameters for system
    // IMPORTANT: add parameters at t he end of the list, otherwise you'll need to reflash the saved params in EEPROM before reading
    sys.cfg.addParam(LOGINT, "Time in ms between log events", "ms", 0, 100000, 250);
    sys.cfg.addParam(LOCALECHO, "When > 0, echo serial input", "", 0, 1, 1);
    sys.cfg.addParam(CMDTIMEOUT, "time in ms before timeout waiting for user input", "ms", 1000, 100000, 10000);
    sys.cfg.addParam(HWPORT0BAUD, "Serial Port 0 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(HWPORT1BAUD, "Serial Port 1 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(STROBEDELAY, "Time between camera trigger and strobe trigger in us", "us", 5, 1000, 50, false);
    sys.cfg.addParam(FRAMERATE, "Camera frame rate in Hz", "Hz", 1, 30, 10, false);
    sys.cfg.addParam(TRIGWIDTH, "Width of the camera trigger pulse in us", "us", 30, 1000000, 100, false);
    sys.cfg.addParam(FLASH1, "Width of LED type 1 flash in us", "us", 50, 1000000, 1000, false);
    sys.cfg.addParam(FLASH2, "Width of LED type 2 flash in us", "us", 50, 1000000, 1000, false);
    sys.cfg.addParam(FLASH3, "Width of LED type 3 flash in us", "us", 50, 1000000, 1000, false);
    sys.cfg.addParam(FLASH4, "Width of LED type 4 flash in us", "us", 50, 1000000, 1000, false);
    sys.cfg.addParam(LOWVOLTAGE, "Voltage in mV where we shut down system", "mV", 10000, 14000, 11500);
    sys.cfg.addParam(STANDBY, "If voltage is low go into standby mode", "", 0, 1, 0);
    sys.cfg.addParam(CHECKHOURLY, "0 = check every minute, 1 = check every hour", "", 0, 1, 0);
    sys.cfg.addParam(STARTUPTIME, "Time in seconds before performing any system checks", "s", 0, 60, 10);
    sys.cfg.addParam(WATCHDOG, "0 = no watchdog, 1 = hardware watchdog timer with 8 sec timeout","", 0, 1, 0);
    sys.cfg.addParam(TEMPLIMIT, "Temerature in C where controller will shutdown and power off camera","C", 0, 80, 55);
    sys.cfg.addParam(HUMLIMIT, "Humidity in % where controller will shutdown and power off camera","%", 0, 100, 60);
    sys.cfg.addParam(MAXSHUTDOWNTIME, "Max time in seconds we wait before cutting power to camera", "s", 15, 600, 60);
    sys.cfg.addParam(CHECKINTERVAL, "Time in seconds between check for bad operating evironment", "s", 10, 3600, 30);

    // configure watchdog timer if enabled
    sys.configWatchdog();

    // Start the remaining serial ports
    HWPORT0.begin(sys.cfg.getInt(HWPORT0BAUD));
    HWPORT1.begin(sys.cfg.getInt(HWPORT1BAUD));

    // Load the last config from EEPROM
    sys.readConfig();
    
}

void loop() {

    sys.update();
    sys.checkInput();
    sys.checkVoltage();
    sys.checkEnv();
    sys.checkCameraPower(); 

    int logInt = sys.cfg.getInt(LOGINT);

    delay(logInt);
    Blink(10, 1);

}
