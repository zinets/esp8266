#pragma once

// typedef enum WeatherCondition {
//   WeatherConditionClear,
//   WeatherConditionRain,
//   WeatherConditionSnow,
//   WeatherConditionSleet,
//   WeatherConditionWind,
//   WeatherConditionFog,
//   WeatherConditionCloudy,
//   WeatherConditionPartlyCloudy,
//   WeatherConditionIndoor,
// } WeatherCondition;

typedef struct WeatherState {
  // WeatherCondition condition;
  bool ready;
  String condition;
  float temperature;
  float pressure;


} WeatherState;
