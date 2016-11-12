#pragma once

typedef struct WeatherState {
  // WeatherCondition condition;
  bool ready;
  String condition;
  float temperature;
  float pressure;
} WeatherState;
