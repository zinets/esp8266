#include "wifiWorker.h"

#define ABS_ZERO -273.0

time_t WiFiWorker::getNtpTime(int timeZone) {
  const int NTP_PACKET_SIZE = 48;
  const char* timerServerDNSName = "0.europe.pool.ntp.org";
  const unsigned int localPort = 8888;

  byte packetBuffer[NTP_PACKET_SIZE];
  IPAddress timeServer;
  WiFiUDP udp;

  time_t result = 0;
  WiFi.hostByName(timerServerDNSName, timeServer);
  // Serial.print("Time server IP: ");
  // Serial.println(timeServer.toString());

  udp.begin(localPort);
  // Serial.print("UDP local port: ");
  // Serial.println(udp.localPort());

  while (udp.parsePacket() > 0);
  // Serial.println("Transmit NTP request");

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

  udp.beginPacket(timeServer, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

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

bool WiFiWorker::parseUrl(String url) {
  WiFiClient client;
  if (!client.connect("api.wunderground.com", 80)) {
    Serial.println("connection failed");
    return false;
  }
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // Serial.println("->");
  // Serial.println(String(ESP.getFreeHeap()));

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.wunderground.com\r\n" +
               "Connection: close\r\n\r\n");
  int retryCounter = 0;
  while(!client.available()) {
    delay(1000);
    retryCounter++;
    if (retryCounter > 10) {
      Serial.print("Connection timeout");
      client.stop();
      return false;
    }
  }

  JsonStreamingParser parser;
  parser.setListener(this);

  int pos = 0;
  boolean isBody = false;
  char c;
  lastWeatherCondition.ready = false;
  lastWeatherCondition.temperature = ABS_ZERO;

  int size = 0;
  client.setNoDelay(false);
  while (client.connected()) {
    while ((size = client.available()) > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
  client.stop();
  // Serial.println("<-");
  // Serial.println(String(ESP.getFreeHeap()));
  return true;
}

bool WiFiWorker::updateWeatherCondition(String apiKey, String query) {
  String q = "/api/" + apiKey + "/conditions/q/" + query + ".json";
  gettingForecast = false;
  return parseUrl(q);
}

bool updateForecast(String apiKey, String query) {
  // String q = "/api/" + apiKey + "/forecast/q/" + query + ".json";
  // dayIndex = 0;
  // forecastBegan = false;
  // gettingForecast = true;
  // return parseUrl(q);
  return 0;
}

void WiFiWorker::updateAstronomy(String apiKey, String query) {
  String q = "/api/" + apiKey + "/astronomy/q/" + query + ".json";
  parseUrl(q);
}

void WiFiWorker::whitespace(char c) {}
void WiFiWorker::startDocument() {
  // Serial.println("start..");
}

void WiFiWorker::startObject() {
  currentParent = currentKey;
}

void WiFiWorker::endObject() {
  currentParent = "";
}

void WiFiWorker::key(String key) {
  // Serial.println("key = " + key);
  currentKey = key;

  if (key == "simpleforecast") {
    forecastBegan = true;
  }
}

void WiFiWorker::value(String value) {
  // Serial.println("val = " + value);
  if (currentKey == "temp_c") {
    lastWeatherCondition.temperature = value.toFloat();
    // мне кажется поле icon содержит уже нужное значение и нет смысла вырезать его из урла
  // } else if (currentKey == "icon_url") {
  //   // Serial.println(value);
  //   int startIndex = value.lastIndexOf("/");
  //   int endIndex = value.indexOf(".gif");
  //   if (startIndex > -1 && endIndex > -1) {
  //     lastWeatherCondition.condition = value.substring(startIndex + 1, endIndex);
  //   }
  } if (currentKey == "icon") {
    if (gettingForecast) {

    } else {
      lastWeatherCondition.condition = value;
    }
  } else if (currentKey == "pressure_in") {
    lastWeatherCondition.pressure = value.toFloat() * 25.4; // ??
  } else if (currentKey == "celsius") {
    if (currentParent == "high") {

    } else if (currentParent == "low") {

    }
  }



  lastWeatherCondition.ready = lastWeatherCondition.temperature > ABS_ZERO &&
  lastWeatherCondition.pressure > 0 &&
  lastWeatherCondition.condition.length() > 0;
}

void WiFiWorker::endArray() {
  // Serial.println("arr end");
}

void WiFiWorker::endDocument() {
 Serial.println("doc end");
}

void WiFiWorker::startArray() {
  // Serial.println("arr start");
}

WeatherState WiFiWorker::getCurrentState() {
  // Serial.print("curent state ready" + String(lastWeatherCondition.ready));
  return lastWeatherCondition;
}
