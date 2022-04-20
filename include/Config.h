#ifndef _CONFIG

#define _CONFIG

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function


// Define GPIOs
#define GPIO_1_IO 42
#define GPIO_2_IO SWIO
#define GPIO_3_IO SWCLK

// Define Triggers
#define TRIG_4_0 A1
#define TRIG_1_0 8
#define TRIG_0_0 4

// Define indicator LEDs
#define TRIG_1_1 9
#define TRIG_4_1 A2 

// Define LED Triggers
#define LED_TRIG1 12
#define LED_TRIG2 11
#define LED_TRIG3 10
#define LED_TRIG4 7

// Define Trigger pins
#define CAM_TRIG TRIG_0_0

// Define dimming pins
#define DIMMING_CS 13

// Define power supply controls
#define LASER_ENABLE 2
#define CAMERA_POWER 6
#define INDCOLOR TRIG_1_1
#define INDMACRO TRIG_4_1

#define DEBUGPORT Serial
#define HWPORT0 Serial0
#define HWPORT1 Serial

#define UI1 HWPORT0
#define JETSONPORT HWPORT1
#define RBRPORT HWPORT3

// Define Config Settings
#define LOGINT "LOGINT"
#define POLLFREQ "POLLFREQ"
#define LOCALECHO "LOCALECHO"
#define CMDTIMEOUT "CMDTIMEOUT"
#define HWPORT0BAUD "HWPORT0BAUD"
#define HWPORT1BAUD "HWPORT1BAUD"
#define STROBEDELAY "STROBEDELAY"
#define FRAMERATE "FRAMERATE"
#define TRIGWIDTH "TRIGWIDTH"
#define FLASH1  "FLASH1"
#define FLASH2  "FLASH2"
#define FLASH3  "FLASH3"
#define FLASH4  "FLASH4"
#define LOWVOLTAGE "LOWVOLTAGE"
#define STANDBY "STANDBY"
#define CHECKHOURLY "CHECKHOURLY"
#define STARTUPTIME "STARTUPTIME"
#define WATCHDOG "WATCHDOG"
#define CAMGUARD "CAMGUARD"
#define TEMPLIMIT "TEMPLIMIT"
#define HUMLIMIT "HUMLIMIT"
#define MAXSHUTDOWNTIME "MAXSHUTDOWNTIME"
#define CHECKINTERVAL "CHECKINTERVAL"

// Define Commands
#define CFG "CFG"
#define PORTPASS "PORTPASS"
#define SETTIME "SETTIME"
#define WRITECONFIG "WRITECONFIG"
#define READCONFIG "READCONFIG"
#define CAMERAON "CAMERAON"
#define CAMERAOFF "CAMERAOFF"
#define SHUTDOWNJETSON "SHUTDOWNJETSON"
#define NEWEVENT "NEWEVENT"
#define PRINTEVENTS "PRINTEVENTS"
#define CLEAREVENTS "CLEAREVENTS"
#define STROBEALL "STROBEALL"
#define STROBE1 "STROBE1"
#define STROBE2 "STROBE2"
#define STROBE3 "STROBE3"
#define STROBE4 "STROBE4"
#define INDCOLORON "INDCOLORON"
#define INDCOLOROFF "INDCOLOROFF"
#define INDMACROON "INDMACROON"
#define INDMACROOFF "INDMACROOFF"
#define LASERON "LASERON"
#define LASEROFF "LASEROFF"


#endif