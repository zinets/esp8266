#include "wifiWorker.h"
#include <U8g2lib.h>
#include "resources.h"

typedef struct IconState {
  String condition;
  const unsigned char *xbm;
} IconState;

const IconState states[] = {
  {.condition = "chanceflurries", .xbm = flurries_bits},
  {.condition = "chancerain", .xbm = chancerain_bits},
  {.condition = "chancesleet", .xbm = chancesnow_bits},
  {.condition = "chancesnow", .xbm = chancesnow_bits},
  {.condition = "chancetstorms", .xbm = chancetstorms_bits},
  {.condition = "clear", .xbm = sunny_bits},
  {.condition = "cloudy", .xbm = cloudy_bits},
  {.condition = "flurries", .xbm = flurries_bits},
  {.condition = "fog", .xbm = fog_bits},
  {.condition = "hazy", .xbm = fog_bits},
  {.condition = "mostlycloudy", .xbm = cloudy_bits},
  {.condition = "mostlysunny", .xbm = mostlysunny_bits},
  {.condition = "nt_chanceflurries", .xbm = flurries_bits},
  {.condition = "nt_chancerain", .xbm = chancerain_bits},
  {.condition = "nt_chancesleet", .xbm = chancesnow_bits},
  {.condition = "nt_chancesnow", .xbm = chancesnow_bits},
  {.condition = "nt_chancetstorms", .xbm = chancetstorms_bits},
  {.condition = "nt_clear", .xbm = nt_clear_bits},
  {.condition = "nt_cloudy", .xbm = cloudy_bits},
  {.condition = "nt_flurries", .xbm = flurries_bits},
  {.condition = "nt_fog", .xbm = fog_bits},
  {.condition = "nt_hazy", .xbm = fog_bits},
  {.condition = "nt_mostlycloudy", .xbm = cloudy_bits},
  {.condition = "nt_mostlysunny", .xbm = mostlysunny_bits},
  {.condition = "nt_partlycloudy", .xbm = nt_partlycloudy_bits},
  {.condition = "nt_partlysunny", .xbm = nt_partlycloudy_bits},
  {.condition = "nt_rain", .xbm = rain_bits},
  {.condition = "nt_sleet", .xbm = snow_bits},
  {.condition = "nt_snow", .xbm = snow_bits},
  {.condition = "nt_sunny", .xbm = nt_clear_bits},
  {.condition = "nt_tstorms", .xbm = tstorms_bits},
  {.condition = "partlycloudy", .xbm = mostlysunny_bits},
  {.condition = "partlysunny", .xbm = mostlysunny_bits},
  {.condition = "rain", .xbm = rain_bits},
  {.condition = "sleet", .xbm = snow_bits},
  {.condition = "snow", .xbm = snow_bits},
  {.condition = "sunny", .xbm = sunny_bits},
  {.condition = "tstorms", .xbm = tstorms_bits},
};




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


  udp->beginPacket(address, 123);
  udp->write(packetBuffer, NTP_PACKET_SIZE);
  udp->endPacket();
}

time_t WiFiWorker::getNtpTime(int timeZone) {
  time_t result = 0;
  WiFi.hostByName(timerServerDNSName, timeServer);
  // Serial.print("Time server IP: ");
  // Serial.println(timeServer.toString());

  udp = new WiFiUDP();
  udp->begin(localPort);
  // Serial.print("UDP local port: ");
  // Serial.println(udp.localPort());

  while (udp->parsePacket() > 0) ; // discard any previously received packets
  // Serial.println("Transmit NTP request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp->parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      // Serial.println("Receive NTP response");
      udp->read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      #define SECS_PER_HOUR (60 * 60)
      // Serial.println("since 1900 - " + String(secsSince1900));
      result = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      // Serial.println("result - " + String(result));
      // Serial.println("result - " + String(secsSince1900 - 2208988800UL));
      Serial.println("NTP time received");

      break;
    }
  }

  if (result == 0) {
    Serial.println("No NTP response");
  }
  return result;
}

void WiFiWorker::parseUrl(String url) {
  WiFiClient client;
  if (!client.connect("api.wunderground.com", 80)) {
    Serial.println("connection failed");
    return;
  }
  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.wunderground.com\r\n" +
               "Connection: close\r\n\r\n");
  int retryCounter = 0;
  while(!client.available()) {
    delay(1000);
    retryCounter++;
    if (retryCounter > 10) {
      Serial.print("Connection timeout");
      return;
    }
  }

  JsonStreamingParser parser;
  parser.setListener(this);

  int pos = 0;
  boolean isBody = false;
  char c;

  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
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
void WiFiWorker::startDocument() {
  // Serial.println("start..");
}
void WiFiWorker::key(String key) {
  // Serial.println("key = " + key);
  currentKey = key;
}
void WiFiWorker::value(String value) {
  if (currentKey == "temp_c") {
    lastWeatherCondition.temperature = value.toFloat();
  } else if (currentKey == "icon_url") {
    String url = value;
    Serial.println(value);
    int startIndex = url.lastIndexOf("/");
    if (startIndex > -1) {
      // Serial.println("start index" + String(startIndex + 1));
      // Serial.println("substr" + value.substring(startIndex + 1));
      lastWeatherCondition.condition = value.substring(startIndex + 1);
      Serial.println(lastWeatherCondition.condition);

      String url = value.substring(startIndex + 1);



    }
  } else if (currentKey == "pressure_in") {
    lastWeatherCondition.pressure = value.toFloat() * 25.4; // ??
  }
  lastWeatherCondition.ready = lastWeatherCondition.temperature > 0 &&
  lastWeatherCondition.pressure > 0 &&
  lastWeatherCondition.condition.length() > 0;
}
void WiFiWorker::endArray() {
  // Serial.println("arr end");
}
void WiFiWorker::endObject() {
  // Serial.println("obj end");
}
void WiFiWorker::endDocument() {
  // Serial.println("doc end");
}
void WiFiWorker::startArray() {
  // Serial.println("arr start");
}
void WiFiWorker::startObject() {
  // Serial.println("obj start");
}

WeatherState WiFiWorker::getCurrentState() {
  return lastWeatherCondition;
}
