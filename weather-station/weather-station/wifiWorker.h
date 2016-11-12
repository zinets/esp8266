#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <JsonListener.h>
#include <JsonStreamingParser.h>

#include "weather-defines.h"

class WiFiWorker : public JsonListener {
private:
  WeatherState lastWeatherCondition;
  String currentKey;
  void parseUrl(String url);
public:
  // updates
  time_t getNtpTime(int timeZone);
  void updateWeatherCondition(String apiKey, String query);
  void updateAstronomy(String apiKey, String query);

  // getters
  WeatherState getCurrentState();

  // listener
  virtual void whitespace(char c);
  virtual void startDocument();
  virtual void key(String key);
  virtual void value(String value);
  virtual void endArray();
  virtual void endObject();
  virtual void endDocument();
  virtual void startArray();
  virtual void startObject();
};
