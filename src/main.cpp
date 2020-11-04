#include <SPIFlash.h>    //get it here: https://github.com/LowPowerLab/SPIFlash
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA260.h>

#include "SystemControl.h"

Adafruit_BME280 bme; // I2C
Adafruit_INA260 ina260_a = Adafruit_INA260();
Adafruit_INA260 ina260_b = Adafruit_INA260();

#include "Utils.h"

#define SEALEVELPRESSURE_HPA (1013.25)

SystemControl sys((Stream*)&Serial);

void setup() {
  Serial.begin(115200);
  // Wait until serial port is opened
  while (!Serial) { delay(10); }

  Serial.println("Adafruit INA260 Test");

  if (!ina260_a.begin()) {
    Serial.println("Couldn't find INA260 A chip");
    while (1);
  }

  Serial.println("Found INA260 A chip");

  if (!ina260_b.begin(0x41)) {
    Serial.println("Couldn't find INA260 B chip");
    while (1);
  }
  Serial.println("Found INA260 B chip");

  unsigned status;
  
  // default settings
  status = bme.begin();  
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }
}

void loop() {

  float current = ina260_a.readCurrent();
  float voltage = ina260_a.readBusVoltage();
  float power = ina260_a.readPower();

  String output = "$PWR_A," + String(current) + "," + String(voltage) + "," + String(power);
  Serial.println(output);

  current = ina260_b.readCurrent();
  voltage = ina260_b.readBusVoltage();
  power = ina260_b.readPower();

  output = "$PWR_B," + String(current) + "," + String(voltage) + "," + String(power);
  Serial.println(output);

  printEnv();

  sys.checkInput();

  delay(500);

}
