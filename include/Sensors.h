#ifndef _SENSORS

#define _SENSORS

#define N_POWER_METERS 7

#define INA260_PWR1_ADDR 0x40
#define INA260_PWR2_ADDR 0x45
#define INA260_5V_ADDR 0x41
#define INA260_LED1_ADDR 0x42
#define INA260_LED2_ADDR 0x43
#define INA260_LED3_ADDR 0x44
#define INA260_12V_ADDR 0x48

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA260.h>

#include "Config.h"

Adafruit_BME280 _bme; // I2C

class Sensors {

    private:
        bool sensorsValid;
   
    public:

        int nPowerMeters;
        unsigned char addresses[N_POWER_METERS];
        float voltage[N_POWER_METERS];
        float current[N_POWER_METERS];
        float power[N_POWER_METERS];
        Adafruit_INA260 * meters[N_POWER_METERS];
        float temperature;
        float pressure;
        float humidity;

        Sensors() {

            sensorsValid = false;
            nPowerMeters = N_POWER_METERS;
            addresses[0] = INA260_PWR1_ADDR;
            addresses[1] = INA260_PWR2_ADDR;
            addresses[2] = INA260_5V_ADDR;
            addresses[3] = INA260_LED1_ADDR;
            addresses[4] = INA260_LED2_ADDR;
            addresses[5] = INA260_LED3_ADDR;
            addresses[6] = INA260_12V_ADDR;


        }

        bool begin() {
            
            sensorsValid = true;

            for (int i = 0; i < N_POWER_METERS; i++) {
                char output[256];
                meters[i] =  new Adafruit_INA260();
                if (!meters[i]->begin(addresses[i])) {
                    sprintf(output, "Couldn't find INA260 Chip #%d at address 0x%x", i, addresses[i]);
                    DEBUGPORT.println(output);
                    sensorsValid = false;
                }
                else {
                    sprintf(output, "Found INA260 Chip #%d at address 0x%x", i, addresses[i]);
                    DEBUGPORT.println(output);
                    // set the number of samples to average
                    meters[i]->setAveragingCount(INA260_COUNT_128);
                    // set the time over which to measure the current and bus voltage
                    meters[i]->setVoltageConversionTime(INA260_TIME_558_us);
                    meters[i]->setCurrentConversionTime(INA260_TIME_558_us);
                }
            }
            
            
            // default settings
            int status = _bme.begin(0x76);  
            // You can also pass in a Wire library object like &Wire2
            // status = bme.begin(0x76, &Wire2)
            if (!status) {
                DEBUGPORT.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
                DEBUGPORT.print("SensorID was: 0x"); Serial.println(_bme.sensorID(),16);
                DEBUGPORT.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
                DEBUGPORT.print("   ID of 0x56-0x58 represents a BMP 280,\n");
                DEBUGPORT.print("        ID of 0x60 represents a BME 280.\n");
                DEBUGPORT.print("        ID of 0x61 represents a BME 680.\n");
                // while (1) delay(10);
                sensorsValid = false;
            }

            return sensorsValid;

        }

        void update() {
            if (!sensorsValid)
                return;
            temperature = _bme.readTemperature();
            pressure = _bme.readPressure();
            humidity = _bme.readHumidity();

            for (int i = 0; i < N_POWER_METERS; i ++) {
                current[i] = meters[i]->readCurrent();
                voltage[i] = meters[i]->readBusVoltage();
                power[i] = meters[i]->readPower();
            }

        }

        void printEnv() {
            if (!sensorsValid)
                return;
            temperature = _bme.readTemperature();
            pressure = _bme.readPressure();
            humidity = _bme.readHumidity();
            String output = "$BME280," + String(temperature) + "," + String(pressure) + "," + String(humidity);
            UI1.println(output);
            UI2.println(output);
        }

        void printPower() {
            if (!sensorsValid)
                return;
            String output;
            UI1.println("\rn");
            UI2.println("\rn");
            DEBUGPORT.println("\rn");
            for (int i = 0; i < N_POWER_METERS; i++) {
                output = "$PWR_" + String(i) + " : " + String(voltage[i] / 1000.0) + "," + String(current[i] / 1000.0) + "," + String(power[i] / 1000.0);
                UI1.println(output);
                UI2.println(output);
                DEBUGPORT.println(output);
            }
            
        }
};

#endif