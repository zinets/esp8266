#include <Arduino.h>

#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "resources.h"

typedef enum WeatherCondition {
  WeatherConditionClear,
  WeatherConditionRain,
  WeatherConditionSnow,
  WeatherConditionSleet,
  WeatherConditionWind,
  WeatherConditionFog,
  WeatherConditionCloudy,
  WeatherConditionPartlyCloudy,
  WeatherConditionIndoor,
} WeatherCondition;

typedef struct WeatherState {
  WeatherCondition condition;
  float temperature;
  float pressure;
} WeatherState;

class Display {
private:
  U8G2 u8g2;
  time_t currentTime;
  WeatherState currentWeatherState;
  WeatherState currentIndoorState;
  // WeatherState forecast[3];
public:
  Display();

  void setTime(time_t dt) {currentTime = dt;};
  void setWeather(WeatherState newState) {currentWeatherState = newState;};
  void setIndoorState(WeatherState newState) {currentIndoorState = newState;};

  void showTime();
  void showCurrentWeather();
  void showIndorState();
};
