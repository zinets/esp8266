#include "barometer.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

void Barometer::adjustData() {
  Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
  if (!bmp.begin()) {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    bmp.getTemperature(&temperature);
    pressure = event.pressure;
    Serial.println("Barometer::adjustData()");
    dataIsReady = true;
  } else {
    Serial.println("Sensor error");

    dataIsReady = false;
  }
}

String Barometer::getPressure() {
  return String(round(pressure * 0.75006)) + " mm";
}

String Barometer::getTemperature() {
  return String(round(temperature)) + " C";
}
