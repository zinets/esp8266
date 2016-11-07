#include "wifiWorker.h"

WiFiWorker::WiFiWorker() {
  udp = WiFiUDP();
}

void WiFiWorker::sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

time_t WiFiWorker::getNtpTime(int timeZone) {
  WiFi.hostByName(timerServerDNSName, timeServer);
  // Serial.print("Time server IP: ");
  // Serial.println(timeServer.toString());

  udp.begin(localPort);
  // Serial.print("UDP local port: ");
  // Serial.println(udp.localPort());

  while (udp.parsePacket() > 0) ; // discard any previously received packets
  // Serial.println("Transmit NTP request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      // Serial.println("Receive NTP response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      #define SECS_PER_HOUR (60 * 60)
      // Serial.println("since 1900 - " + String(secsSince1900));
      time_t result = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      // Serial.println("result - " + String(result));
      // Serial.println("result - " + String(secsSince1900 - 2208988800UL));
      Serial.println("NTP time received");
      return result;
    }
  }

  Serial.println("No NTP response");
  return 0;
}

void parseUrl(String url) {

}

void WiFiWorker::updateWeatherCondition(String apiKey, String query) {
  String q = "/api/" + apiKey + "/conditions/q/" + query + ".json";
  parseUrl(q);
}

void WiFiWorker::updateAstronomy(String apiKey, String query) {
  String q = "/api/" + apiKey + "/astronomy/q/" + query + ".json";
  parseUrl(q);
}

void WiFiWorker::whitespace(char c) {}
void WiFiWorker::startDocument() {}
void WiFiWorker::key(String key) {}
void WiFiWorker::value(String value) {}
void WiFiWorker::endArray() {}
void WiFiWorker::endObject() {}
void WiFiWorker::endDocument() {}
void WiFiWorker::startArray() {}
void WiFiWorker::startObject() {}
