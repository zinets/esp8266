#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <JsonListener.h>
#include <JsonStreamingParser.h>


const int NTP_PACKET_SIZE = 48;

class WiFiWorker : public JsonListener {
private:
  const char* timerServerDNSName = "0.europe.pool.ntp.org";
  const unsigned int localPort = 8888;

  byte packetBuffer[NTP_PACKET_SIZE];
  IPAddress timeServer;
  WiFiUDP *udp;

  String currentKey;

  void sendNTPpacket(IPAddress &address);
  void parseUrl(String url);
public:
  // updates
  time_t getNtpTime(int timeZone);
  void updateWeatherCondition(String apiKey, String query);
  void updateAstronomy(String apiKey, String query);

  // getters
  // WeatherCondition getCurrentState();

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
