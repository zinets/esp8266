#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int NTP_PACKET_SIZE = 48;

class WiFiWorker {
private:
  const char* timerServerDNSName = "0.europe.pool.ntp.org";
  const unsigned int localPort = 8888;

  byte packetBuffer[NTP_PACKET_SIZE];
  IPAddress timeServer;
  WiFiUDP udp;

  void sendNTPpacket(IPAddress &address);
  // void parseUrl(String url);
public:
  WiFiWorker();

  time_t getNtpTime(int timeZone);
  void updateWeatherCondition(String apiKey, String query);
};
