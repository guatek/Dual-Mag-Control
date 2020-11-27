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

// Define GPIOs
#define GPIO_1_IO AREF
#define GPIO_2_IO SWIO
#define GPIO_3_IO SWCLK

// Define Triggers
#define TRIG_4_0 A1
#define TRIG_1_0 8
#define TRIG_0_0 4

// Define Trigger pins
#define HIGH_MAG_CAM_TRIG GPIO_2_IO
#define LOW_MAG_CAM_TRIG GPIO_3_IO
#define HIGH_MAG_STROBE_TRIG TRIG_0_0
#define LOW_MAG_STROBE_TRIG TRIG_4_0
#define FLASH_TYPE_PIN TRIG_1_0

#define DEBUGPORT Serial
#define HWPORT0 Serial0
#define HWPORT1 Serial1
#define HWPORT2 Serial2
#define HWPORT3 Serial3

#define UI1 HWPORT0
#define UI2 HWPORT2
#define JETSONPORT HWPORT1
#define RBRPORT HWPORT3


#endif