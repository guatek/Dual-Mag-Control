#ifndef _CONFIG

#define _CONFIG

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function

// Define additional serial ports

// Serial2
#define PIN_SERIAL2_RX       (5ul)
#define PIN_SERIAL2_TX       (2ul)
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)

// Serial3
#define PIN_SERIAL3_RX       (12ul)
#define PIN_SERIAL3_TX       (10ul)
#define PAD_SERIAL3_TX       (UART_TX_PAD_2)
#define PAD_SERIAL3_RX       (SERCOM_RX_PAD_3)

// Serial objects
Uart Serial2( &sercom2, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX ) ;
Uart Serial3( &sercom1, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX ) ;

// Set SERCOM peripherals
void configSerialPins() {
    pinPeripheral(5, PIO_SERCOM);
    pinPeripheral(2, PIO_SERCOM);
    pinPeripheral(12, PIO_SERCOM);
    pinPeripheral(10, PIO_SERCOM);
}

// Serial handlers
void SERCOM2_Handler()
{
  Serial2.IrqHandler();
}

void SERCOM1_Handler()
{
  Serial3.IrqHandler();
}

// Define power control
#define V12_ENABLE A2
#define LED3_ENABLE A1
#define LED2_ENABLE 9
#define LED1_ENABLE 11

// Define GPIOs
#define GPIO_1_IO 42
#define GPIO_2_IO SWIO
#define GPIO_3_IO SWCLK

// Define Triggers
#define TRIG_4_0 7
#define TRIG_1_0 6
#define TRIG_0_0 4

// Define strobe pins
#define COLOR_FLASH 7
#define IR_FLASH 6
#define VIOLET_FLASH A0

// Inputs and UVC
#define UVC_ENABLE PIN_ATN
#define COLOR_TRIG_IN PIN_PA03
#define IR_TRIG_IN PIN_PA31
#define VIOLET_TRIG_IN PIN_PA30
#define UVC_TRIG_IN PIN_PA08

// Define Trigger pins
#define HIGH_MAG_CAM_TRIG GPIO_2_IO
#define LOW_MAG_CAM_TRIG GPIO_3_IO
#define HIGH_MAG_STROBE_TRIG COLOR_FLASH
#define LOW_MAG_STROBE_TRIG IR_FLASH

// Define SD CARD PINS
#define SDCARD_DETECT GPIO_1_IO
#define SDCARD_ENABLE 11
#define SDCARD_CS 13

#define DEBUGPORT Serial
#define HWPORT0 Serial0
#define HWPORT1 Serial1
#define HWPORT2 Serial2
#define HWPORT3 Serial3

#define UI1 HWPORT0
#define UI2 HWPORT1
#define JETSONPORT HWPORT2
#define RBRPORT HWPORT3

// Define Config Settings
#define LOGINT "LOGINT"
#define POLLFREQ "POLLFREQ"
#define DEPTHCHECKINTERVAL "DEPTHCHECKINTERVAL"
#define DEPTHTHRESHOLD "DEPTHTHRESHOLD"
#define LOCALECHO "LOCALECHO"
#define CMDTIMEOUT "CMDTIMEOUT"
#define HWPORT0BAUD "HWPORT0BAUD"
#define HWPORT1BAUD "HWPORT1BAUD"
#define HWPORT2BAUD "HWPORT2BAUD"
#define HWPORT3BAUD "HWPORT3BAUD"
#define STROBEDELAY "STROBEDELAY"
#define FRAMERATE "FRAMERATE"
#define TRIGWIDTH "TRIGWIDTH"
#define LOWMAGCOLORFLASH "LOWMAGCOLORFLASH"
#define LOWMAGREDFLASH "LOWMAGREDFLASH"
#define HIGHMAGCOLORFLASH "HIGHMAGCOLORFLASH"
#define HIGHMAGREDFLASH "HIGHMAGREDFLASH"
#define VIOLETFLASH "VIOLETFLASH"
#define FLASHTYPE "FLASHTYPE"
#define PROFILEMODE "PROFILEMODE"
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
#define MINDEPTH "MINDEPTH"
#define MAXDEPTH "MAXDEPTH"
#define ECHORBR "ECHORBR"
#define USERBRCLOCK "USERBRCLOCK"
#define CTDTYPE "CTDTYPE"
#define UVCENABLE "UVCENABLE"
#define UVCFREQ "UVCFREQ"
#define UVCDUTY "UVCDUTY"

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
#define GOTOSLEEP "GOTOSLEEP"
#define PRINTPOWER "PRINTPOWER"
#define UVCON "UVCON"
#define UVCOFF "UVCOFF"


#endif