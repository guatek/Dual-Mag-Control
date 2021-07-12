#include "Config.h"
#include "SystemControl.h"
#include "Sensors.h"
#include "SystemTrigger.h"
#include "SystemConfig.h"
#include "Utils.h"

// Global system control variable
SystemControl sys;

// High Mag Trigger Callback
void TriggerCallback()
{
    digitalWrite(LEFT_CAM_TRIG,HIGH);
    digitalWrite(RIGHT_CAM_TRIG,HIGH);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(STROBE_TRIG,HIGH);
    if (sys.strobeDuration-FLASH_DELAY_OFFSET >= MIN_FLASH_DURATION)
        delayMicroseconds(sys.strobeDuration-FLASH_DELAY_OFFSET);
    digitalWrite(STROBE_TRIG,LOW);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(LEFT_CAM_TRIG,LOW);
    digitalWrite(RIGHT_CAM_TRIG,LOW);
}

// Wrapper for updaing timers and flashes from callback functions
void setTriggers() {
    sys.setTriggers();
}

void setFlashes() {
    sys.configureFlashDurations();
}

// wrapper for turning system on
void turnOnCamera() {
    if (sys.cfg.getInt("PROFILEMODE") == 1) {
        sys.turnOnCamera();
    }
}



void setup() {

    // Imediately turn off camera and strobe ports in case we hang on other
    // parts of the setup
    pinMode(CAMERA_POWER, OUTPUT);
    pinMode(STROBE_POWER, OUTPUT);
    digitalWrite(CAMERA_POWER, LOW);
    digitalWrite(STROBE_POWER, HIGH);

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
    sys.cfg.addParam(HWPORT2BAUD, "Serial Port 2 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(HWPORT3BAUD, "Serial Port 3 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(STROBEDELAY, "Time between camera trigger and strobe trigger in us", "us", 5, 1000, 50, false, setFlashes);
    sys.cfg.addParam(FRAMERATE, "Camera frame rate in Hz", "Hz", 1, 30, 10, false, setTriggers);
    sys.cfg.addParam(TRIGWIDTH, "Width of the camera trigger pulse in us", "us", 30, 10000, 100, false, setFlashes);
    sys.cfg.addParam(FLASH, "Width of the low-mag white flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam(OPMODE,"0 = upcast only, 1 = always on", "", 0, 1, 0);
    sys.cfg.addParam(LOWVOLTAGE, "Voltage in mV where we shut down system", "mV", 10000, 14000, 11500);
    sys.cfg.addParam(STANDBY, "If voltage is low go into standby mode", "", 0, 1, 0);
    sys.cfg.addParam(CHECKHOURLY, "0 = check every minute, 1 = check every hour", "", 0, 1, 0);
    sys.cfg.addParam(STARTUPTIME, "Time in seconds before performing any system checks", "s", 0, 60, 10);
    sys.cfg.addParam(WATCHDOG, "0 = no watchdog, 1 = hardware watchdog timer with 8 sec timeout","", 0, 1, 0);
    sys.cfg.addParam(CAMGUARD,"Time guard between power ON/OFF events in seconds", "s", 10, 120, 30);
    sys.cfg.addParam(TEMPLIMIT, "Temerature in C where controller will shutdown and power off camera","C", 0, 80, 55);
    sys.cfg.addParam(HUMLIMIT, "Humidity in % where controller will shutdown and power off camera","%", 0, 100, 60);
    sys.cfg.addParam(MAXSHUTDOWNTIME, "Max time in seconds we wait before cutting power to camera", "s", 15, 600, 60);
    sys.cfg.addParam(CHECKINTERVAL, "Time in seconds between check for bad operating evironment", "s", 10, 3600, 30);

    // configure watchdog timer if enabled
    sys.configWatchdog();

    // Start the remaining serial ports
    HWPORT0.begin(sys.cfg.getInt(HWPORT0BAUD));
    HWPORT1.begin(sys.cfg.getInt(HWPORT1BAUD));
    HWPORT2.begin(sys.cfg.getInt(HWPORT2BAUD));
    HWPORT3.begin(sys.cfg.getInt(HWPORT3BAUD));

    // Config the SERCOM muxes AFTER starting the ports
    configSerialPins();

    // Load the last config from EEPROM
    sys.readConfig();

    sys.loadScheduler();

    // Setup flashes triggers and polling
    setFlashes();
    setTriggers();
    
}

void loop() {

    sys.update();
    sys.checkInput();
    sys.checkVoltage();
    sys.checkEnv();
    sys.checkEvents();
    sys.checkCameraPower(); 

    int logInt = sys.cfg.getInt(LOGINT);

    delay(logInt);
    Blink(10, 1);

}
