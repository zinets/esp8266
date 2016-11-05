#include "temperature.h"

Temperature::Temperature() {
  bmpSensor = Adafruit_BMP085_Unified(10085);
  if (!bmpSensor.begin()) {
    Serial.println("Hardware problem with BMP180?");

    while (1);
  }
}

bool Temperature::canShowData() {
  return dataIsReady;
}

void Temperature::adjustData() {
  sensors_event_t event;
  bmpSensor.getEvent(&event);

  if (event.pressure) {
    pressure = event.pressure;
    bmpSensor.getTemperature(&temperature);

    dataIsReady = true;
  } else {
    dataIsReady = false;
  }
}
