#include <Arduino.h>

#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "resources.h"
#include "weather-defines.h"

class Display {
private:
  U8G2 u8g2;

  WeatherState currentWeatherState;
  WeatherState currentIndoorState;
  // WeatherState forecast[3];
public:
  Display();

  void setWeather(WeatherState newState) {currentWeatherState = newState;};
  void setIndoorState(WeatherState newState) {currentIndoorState = newState;};

  // screens
  void showStartupScreen();
  void showTimeScreen(String currentTime, String currentDate);
  void showNYRemainTime(String remainingTime);
  void showIndoorData(String currentTemperature, String currentPressure);
  
  void showConfigData(String apName);

  void showCurrentWeather();
  void showIndorState();

  // forecast
  // NY remaining
};
