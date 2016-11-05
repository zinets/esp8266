#include "udp_time.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* timerServerDNSName = "0.europe.pool.ntp.org";
const unsigned int localPort = 8888;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

IPAddress timeServer;
WiFiUDP Udp;

void sendNTPpacket(IPAddress &address) {
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

  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime(int timeZone) {
  WiFi.hostByName(timerServerDNSName, timeServer);
  Serial.print("Time server IP: ");
  Serial.println(timeServer.toString());

  Udp.begin(localPort);
  Serial.print("UDP local port: ");
  Serial.println(Udp.localPort());

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      #define SECS_PER_HOUR (60 * 60)
      Serial.println("since 1900 - " + String(secsSince1900));
      time_t result = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      Serial.println("result - " + String(result));
      Serial.println("result - " + String(secsSince1900 - 2208988800UL));

      return result;
    }
  }

  Serial.println("No NTP response");
  return 0;
}
