#pragma once

typedef struct WeatherState {
  // WeatherCondition condition;
  bool ready;
  String condition;
  float temperature;
  float maxTemperature;
  float minTemperature;
  float pressure;
} WeatherState;
