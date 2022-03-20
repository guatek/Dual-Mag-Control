#ifndef _SENSORS

#define _SENSORS

#define INA260_SYSTEM_ADDR 0x40
#define INA260_CAMERA_ADDR 0x44
#define INA260_LED_ADDR 0x45

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA260.h>

#include "Config.h"

Adafruit_BME280 _bme; // I2C
Adafruit_INA260 _ina260_a = Adafruit_INA260();
Adafruit_INA260 _ina260_b = Adafruit_INA260();
Adafruit_INA260 _ina260_c = Adafruit_INA260();

class Sensors {

    private:
        bool sensorsValid;
   
    public:

        float voltage[3];
        float current[3];
        float power[3];
        float temperature;
        float pressure;
        float humidity;

        Sensors() {
            sensorsValid = false;

        }

        bool begin() {

            sensorsValid = true;
            
            if (!_ina260_a.begin(INA260_SYSTEM_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 A chip");
                sensorsValid = false;
            }

            if (!_ina260_b.begin(INA260_CAMERA_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 B chip");
                sensorsValid = false;
            }

            if (!_ina260_c.begin(INA260_LED_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 C chip");
                sensorsValid = false;
            }
            
            
            // default settings
            int status = _bme.begin(BME280_ADDRESS_ALTERNATE);  
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
            current[0] = _ina260_a.readCurrent();
            voltage[0] = _ina260_a.readBusVoltage();
            power[0] = _ina260_a.readPower();
            current[1] = _ina260_b.readCurrent();
            voltage[1] = _ina260_b.readBusVoltage();
            power[1] = _ina260_b.readPower();
            current[2] = _ina260_c.readCurrent();
            voltage[2] = _ina260_c.readBusVoltage();
            power[2] = _ina260_c.readPower();
        }

        void printEnv() {
            if (!sensorsValid)
                return;
            temperature = _bme.readTemperature();
            pressure = _bme.readPressure();
            humidity = _bme.readHumidity();
            String output = "$BME280," + String(temperature) + "," + String(pressure) + "," + String(humidity);
            UI1.println(output);
        }

        void printPower() {
            if (!sensorsValid)
                return;
            current[0] = _ina260_a.readCurrent();
            voltage[0] = _ina260_a.readBusVoltage();
            power[0] = _ina260_a.readPower();

            String output = "$PWR_A," + String(current[0]) + "," + String(voltage[0]) + "," + String(power[0]);
            UI1.println(output);

            current[1] = _ina260_b.readCurrent();
            voltage[1] = _ina260_b.readBusVoltage();
            power[1] = _ina260_b.readPower();

            output = "$PWR_B," + String(current[1]) + "," + String(voltage[1]) + "," + String(power[1]);
            UI1.println(output);
            
        }
};

#endif