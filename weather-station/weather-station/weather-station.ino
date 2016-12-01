#include <Arduino.h>
// i2c
#include <Wire.h>
// WiFi
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Display
#include "display.h"
#include "clock.h"
#include "barometer.h"
#include "wifiWorker.h"

#define TIME_ZONE +2
#define API_KEY "d85b09d20c0e17b4"
#define LOCATION "ur/zaporizhzhya"

Display *display;
Clock *clock;
Barometer *barometer;
WiFiWorker *worker;

void showTimeScreen();
void showTemperatureScreen();
void showNYRemainTime();
void showCurrentWeatherCondition();
void configModeCallback (WiFiManager *myWiFiManager);

typedef struct Screen {
  void (*displayFunc)();
  int timeout;
  bool enabled;
} Screen;

enum ScreenType {
  ScreenTypeTime,
  ScreenTypeNYRemain,
  ScreenTypeIndoorData,
  ScreenTypeWeather,
};

int const numberOfScreens = 4;
Screen screens[numberOfScreens] = {
  /* ScreenTypeTime */        {.displayFunc = showTimeScreen, .timeout = 6, .enabled = true},
  /* ScreenTypeNYRemain */    {.displayFunc = showNYRemainTime, .timeout = 3, .enabled = true},
  /* ScreenTypeIndoorData */  {.displayFunc = showTemperatureScreen, .timeout = 3, .enabled = false},
  /* ScreenTypeWeather */     {.displayFunc = showCurrentWeatherCondition, .timeout = 3, .enabled = false},
};
int currentScreenIndex = 0;

#define DEBUG
#define SDA_PIN 12
#define SCL_PIN 2

// период обновления температуры/давления в минутах
#define UPDATE_BARO_PERIOD (1 * 60 * 1000)
// период обновления времени в часах
#define UPDATE_TIME_PERIOD (1 * 60 * 60 * 1000)
// период обновления погоды (30 min)
#define UPDATE_WEATHER_PERIOD (30 * 60 * 1000)
// период обновления прогноза (10 hours)
#define UPDATE_FORECAST_PERIOD (10 * 60 * 60 * 1000)

typedef struct FLAGS {
  unsigned char isWiFiconnected: 1;

  unsigned char shouldUpdateBaro: 1;
  unsigned char shouldConnectWiFi: 1;
  unsigned char shouldUpdateTime: 1;
  unsigned char shouldUpdateWeather: 1;
  unsigned char shouldUpdateForecast: 1;

  unsigned long nextUpdateBaroTime;
  unsigned long nextUpdateTimeTime;
  unsigned long nextUpdateWeatherTime;
  unsigned long nextUpdateForecastTime;
} FLAGS;
volatile FLAGS flags;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  display = new Display();
  clock = new Clock();
  barometer = new Barometer();
  worker = new WiFiWorker();

  flags.shouldUpdateBaro = true;
  flags.shouldConnectWiFi = true;
}

void connectToWiFi() {
  Serial.println("try to connect to wifi");
  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  if (wifiManager.autoConnect()) {
    flags.isWiFiconnected = true;
    flags.shouldUpdateTime = true;
  } else {
    Serial.println("failed to connect and hit timeout");

    ESP.reset();
    // delay(1000);
    flags.isWiFiconnected = false;
  }
  flags.shouldConnectWiFi = false;

  flags.nextUpdateTimeTime = millis() + 3000;
  flags.nextUpdateWeatherTime = millis() + 5000;
}

void updateTime() {
  time_t now = worker->getNtpTime(TIME_ZONE);
  if (now > 0) {
    clock->adjustDateTime(now);
  }

  flags.shouldUpdateTime = false;
  flags.nextUpdateTimeTime = millis() + UPDATE_TIME_PERIOD;
}

void updateWeather() {
  worker->updateWeatherCondition(API_KEY, LOCATION);

  flags.shouldUpdateWeather = false;
  flags.nextUpdateWeatherTime = millis() + UPDATE_WEATHER_PERIOD;
}

void updateForecast() {

  flags.shouldUpdateForecast = false;
  flags.nextUpdateForecastTime = millis() + UPDATE_FORECAST_PERIOD;
}

void loop() {
  if (flags.shouldUpdateBaro) {
    barometer->adjustData();

    flags.nextUpdateBaroTime = millis() + UPDATE_BARO_PERIOD;
    flags.shouldUpdateBaro = false;
  } else if (flags.shouldConnectWiFi) {
    connectToWiFi();
  } else if (flags.shouldUpdateTime) {
    updateTime();
  } else if (flags.shouldUpdateWeather) {
    if (!updateWeather()) {
      flags.nextUpdateWeatherTime = millis() + 60 * 1000;
      Serial.println("Shedule updating");
    }
  } else if (flags.shouldUpdateForecast) {
    updateForecast();
  }

  switch (currentScreenIndex) {
    case ScreenTypeNYRemain:
      screens[ScreenTypeNYRemain].enabled = clock->canShowNYRemainTime();
      break;
    case ScreenTypeIndoorData:
      screens[ScreenTypeIndoorData].enabled = barometer->canShowData();
      break;
    case ScreenTypeWeather:
      screens[ScreenTypeWeather].enabled = worker->getCurrentState().ready;
      break;
      // todo: готовы ли погода, прогноз etc
    default:
      break;
  }

  if (screens[currentScreenIndex].enabled) {
    for (int x = 0; x < screens[currentScreenIndex].timeout; x++) {
      screens[currentScreenIndex].displayFunc();
      delay(1000);
      if (!screens[currentScreenIndex].enabled) {
        break;
      }
    }
  }
  currentScreenIndex = (++currentScreenIndex) % numberOfScreens;

  unsigned long now = millis();
  flags.shouldUpdateBaro = now > flags.nextUpdateBaroTime;
  flags.shouldUpdateTime = now > flags.nextUpdateTimeTime;
  flags.shouldUpdateWeather = now > flags.nextUpdateWeatherTime || !worker->getCurrentState().ready;
  flags.shouldUpdateForecast = now > flags.nextUpdateForecastTime;

  // Serial.println(":" + String(ESP.getFreeHeap()));
}

void showTimeScreen() {
  display->showTimeScreen(clock->getTime(), clock->getDate());
}

void showNYRemainTime() {
  display->showNYRemainTime(clock->getNYRemainingTime());
}

void showTemperatureScreen() {
  display->showIndoorData(barometer->getTemperature(), barometer->getPressure());
}

void showCurrentWeatherCondition() {
  WeatherState s = worker->getCurrentState();
  display->setWeather(s);
  display->showCurrentWeather();
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  String apName = myWiFiManager->getConfigPortalSSID();
  Serial.println(apName);
  display->showConfigData(apName);
}
