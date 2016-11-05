#include "temperature.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

Temperature::Temperature() {
  Serial.println("Temperature::Temperature()");

  dataIsReady = false;
}

bool Temperature::canShowData() {
  return dataIsReady;
}

void Temperature::adjustData() {
  Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
  if (!bmp.begin()) {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    Serial.println("Measured!");

    bmp.getTemperature(&temperature);
    pressure = event.pressure;

    dataIsReady = true;
  } else {
    Serial.println("Sensor error");
    dataIsReady = false;
  }
}

String Temperature::getPressure() {
  return String(pressure * 0.75006) + " mm";
}

String Temperature::getTemperature() {
  return String(temperature) + " C";
}
