#ifndef _SENSORS

#define _SENSORS

#define INA260_PWR1_ADDR 0x40
#define INA260_PWR2_ADDR 0x45
#define INA260_5V_ADDR 0x41
#define INA260_LED1_ADDR 0x42
#define INA260_LED2_ADDR 0x43
#define INA260_LED3_ADDR 0x44

#define INA260_STROBE_ADDR 0x41

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA260.h>

#include "Config.h"

Adafruit_BME280 _bme; // I2C
Adafruit_INA260 _ina260_a = Adafruit_INA260();
Adafruit_INA260 _ina260_b = Adafruit_INA260();
Adafruit_INA260 _ina260_c = Adafruit_INA260();
Adafruit_INA260 _ina260_d = Adafruit_INA260();
Adafruit_INA260 _ina260_e = Adafruit_INA260();
Adafruit_INA260 _ina260_f = Adafruit_INA260();

class Sensors {

    private:
        bool sensorsValid;
   
    public:

        float voltage[6];
        float current[6];
        float power[6];
        float temperature;
        float pressure;
        float humidity;

        Sensors() {
            sensorsValid = false;

        }

        bool begin() {

            sensorsValid = true;
            
            if (!_ina260_a.begin(INA260_PWR1_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 PWR1 Chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("INA260 PWR1 Chip OK");
                // set the number of samples to average
                _ina260_a.setAveragingCount(INA260_COUNT_256);
                // set the time over which to measure the current and bus voltage
                _ina260_a.setVoltageConversionTime(INA260_TIME_558_us);
                _ina260_a.setCurrentConversionTime(INA260_TIME_558_us);
            }

            if (!_ina260_b.begin(INA260_PWR2_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 PWR2 Chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("INA260 PWR2 Chip OK");
                // set the number of samples to average
                _ina260_b.setAveragingCount(INA260_COUNT_256);
                // set the time over which to measure the current and bus voltage
                _ina260_b.setVoltageConversionTime(INA260_TIME_558_us);
                _ina260_b.setCurrentConversionTime(INA260_TIME_558_us);
            }

            if (!_ina260_c.begin(INA260_5V_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 5V Chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("INA260 5V Chip OK");
                // set the number of samples to average
                _ina260_c.setAveragingCount(INA260_COUNT_256);
                // set the time over which to measure the current and bus voltage
                _ina260_c.setVoltageConversionTime(INA260_TIME_558_us);
                _ina260_c.setCurrentConversionTime(INA260_TIME_558_us);
            }

            if (!_ina260_d.begin(INA260_LED1_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 LED1 Chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("INA260 LED1 Chip OK");
                // set the number of samples to average
                _ina260_d.setAveragingCount(INA260_COUNT_256);
                // set the time over which to measure the current and bus voltage
                _ina260_d.setVoltageConversionTime(INA260_TIME_558_us);
                _ina260_d.setCurrentConversionTime(INA260_TIME_558_us);
            }

            if (!_ina260_e.begin(INA260_LED2_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 LED2 Chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("INA260 LED2 Chip OK");
                // set the number of samples to average
                _ina260_e.setAveragingCount(INA260_COUNT_256);
                // set the time over which to measure the current and bus voltage
                _ina260_e.setVoltageConversionTime(INA260_TIME_558_us);
                _ina260_e.setCurrentConversionTime(INA260_TIME_558_us);
            }

            if (!_ina260_f.begin(INA260_LED3_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 LED3 Chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("INA260 LED3 Chip OK");
                // set the number of samples to average
                _ina260_f.setAveragingCount(INA260_COUNT_256);
                // set the time over which to measure the current and bus voltage
                _ina260_f.setVoltageConversionTime(INA260_TIME_558_us);
                _ina260_f.setCurrentConversionTime(INA260_TIME_558_us);
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
            current[0] = _ina260_a.readCurrent();
            voltage[0] = _ina260_a.readBusVoltage();
            power[0] = _ina260_a.readPower();
            current[1] = _ina260_b.readCurrent();
            voltage[1] = _ina260_b.readBusVoltage();
            power[1] = _ina260_b.readPower();
            current[2] = _ina260_c.readCurrent();
            voltage[2] = _ina260_c.readBusVoltage();
            power[2] = _ina260_c.readPower();
            current[3] = _ina260_d.readCurrent();
            voltage[3] = _ina260_d.readBusVoltage();
            power[3] = _ina260_d.readPower();
            current[4] = _ina260_e.readCurrent();
            voltage[4] = _ina260_e.readBusVoltage();
            power[4] = _ina260_e.readPower();
            current[5] = _ina260_f.readCurrent();
            voltage[5] = _ina260_f.readBusVoltage();
            power[5] = _ina260_f.readPower();
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
            current[0] = _ina260_a.readCurrent();
            voltage[0] = _ina260_a.readBusVoltage();
            power[0] = _ina260_a.readPower();

            String output = "$PWR_A," + String(current[0]) + "," + String(voltage[0]) + "," + String(power[0]);
            UI1.println(output);
            UI2.println(output);

            current[1] = _ina260_b.readCurrent();
            voltage[1] = _ina260_b.readBusVoltage();
            power[1] = _ina260_b.readPower();

            output = "$PWR_B," + String(current[1]) + "," + String(voltage[1]) + "," + String(power[1]);
            UI1.println(output);
            UI2.println(output);
            
        }
};

#endif