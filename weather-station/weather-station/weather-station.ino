#include <Arduino.h>
// i2c
#include <Wire.h>

// Display
#include "display.h"
#include "clock.h"

Display display;
Clock clock;

void showTimeScreen();
void showAnotherScreen();
void showNYRemainTime();

typedef struct Screen {
  void (*displayFunc)();
  int timeout;
  bool enabled;
} Screen;

enum ScreenType {
  ScreenTypeTime,
  ScreenTypeNYRemain,
  ScreenTypeAnother,
};

int const numberOfScreens = 3;
Screen screens[numberOfScreens] = {
  /* ScreenTypeTime */        {.displayFunc = showTimeScreen, .timeout = 2, .enabled = true},
  /* ScreenTypeNYRemain */    {.displayFunc = showNYRemainTime, .timeout = 6, .enabled = true},
  /* ScreenTypeAnother */     {.displayFunc = showAnotherScreen, .timeout = 1, .enabled = true},
};
int currentScreenIndex = 0;


#define DEBUG
#define SDA_PIN 12
#define SCL_PIN 2

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  display = Display();
  clock = Clock();

  screens[ScreenTypeNYRemain].enabled = clock.canShowNYRemainTime();
}



void showTimeScreen() {
  display.showTimeScreen(clock.getTime(), clock.getDate());
}

void showAnotherScreen() {
  display.showStartupScreen();
}

void showNYRemainTime() {
  display.showNYRemainTime(clock.getNYRemainingTime());
}

void loop() {
  switch (currentScreenIndex) {
    case ScreenTypeNYRemain:
      screens[ScreenTypeNYRemain].enabled = clock.canShowNYRemainTime();
      break;
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
}
//
// #undef OLD_CODE
// #ifdef OLD_CODE
//
// // wi-fi
// #include <ESP8266WiFi.h>
// #include <DNSServer.h>
// #include <ESP8266WebServer.h>
// #include "WiFiManager.h"
// #include <WiFiUdp.h>
//
//
//
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BMP085_U.h>
//
//
//
// Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
//
// // UDP - получение точного времени
//
// const int timeZone = 3;               // Central European Time
// const char* timerServerDNSName = "0.europe.pool.ntp.org";
// IPAddress timeServer;
//
// WiFiUDP Udp;
// const unsigned int localPort = 8888;  // local port to listen for UDP packets
// const int NTP_PACKET_SIZE = 48;       // NTP time is in the first 48 bytes of message
// byte packetBuffer[NTP_PACKET_SIZE];
//
// Display display;
//
// void sendNTPpacket(IPAddress &address) {
//   memset(packetBuffer, 0, NTP_PACKET_SIZE);
//   packetBuffer[0] = 0b11100011; // LI, Version, Mode
//   packetBuffer[1] = 0;          // Stratum, or type of clock
//   packetBuffer[2] = 6;          // Polling Interval
//   packetBuffer[3] = 0xEC;       // Peer Clock Precision
//   // 8 bytes of zero for Root Delay & Root Dispersion
//   packetBuffer[12]  = 49;
//   packetBuffer[13]  = 0x4E;
//   packetBuffer[14]  = 49;
//   packetBuffer[15]  = 52;
//
//   Udp.beginPacket(address, 123);
//   Udp.write(packetBuffer, NTP_PACKET_SIZE);
//   Udp.endPacket();
// }
//
// time_t getNtpTime() {
//   while (Udp.parsePacket() > 0) ; // discard any previously received packets
//   Serial.println("Transmit NTP request");
//   sendNTPpacket(timeServer);
//   uint32_t beginWait = millis();
//   while (millis() - beginWait < 1500) {
//     int size = Udp.parsePacket();
//     if (size >= NTP_PACKET_SIZE) {
//       Serial.println("Receive NTP response");
//       Udp.read(packetBuffer, NTP_PACKET_SIZE);
//       unsigned long secsSince1900;
//       // convert four bytes starting at location 40 to a long integer
//       secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
//       secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
//       secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
//       secsSince1900 |= (unsigned long)packetBuffer[43];
//       #define SECS_PER_HOUR 60
//       return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
//     }
//   }
//
//   Serial.println("No NTP response");
//   return 0;
// }
//
// void configModeCallback (WiFiManager *myWiFiManager) {
//   Serial.println("Entered config mode");
//   Serial.println(WiFi.softAPIP());
//   //if you used auto generated SSID, print it
//   Serial.println(myWiFiManager->getConfigPortalSSID());
// }
//
// void printTemperature() {
//   sensors_event_t event;
//   bmp.getEvent(&event);
//
//   /* Display the results (barometric pressure is measure in hPa) */
//   if (event.pressure) {
//     float temperature;
//     bmp.getTemperature(&temperature);
//
//     Serial.print("Temperature: ");
//     Serial.print(temperature);
//     Serial.println(" C");
//   } else {
//     Serial.println("Sensor error");
//   }
// }
//
// void setup() {
//   Serial.begin(115200);
//
//   display = Display();
//   display.showCurrentWeather();
//
//   Wire.begin(12, 2);
//
// while(1);
//
//   if (!bmp.begin()) {
//     /* There was a problem detecting the BMP085 ... check your connections */
//     Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
//     while(1);
//   } else {
//     Serial.println("BMP sensor found");
//   }
//
//   //WiFiManager
//   //Local intialization. Once its business is done, there is no need to keep it around
//   WiFiManager wifiManager;
//   //reset settings - for testing
//   // wifiManager.resetSettings();
//
//   //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
//   wifiManager.setAPCallback(configModeCallback);
//
//   //fetches ssid and pass and tries to connect
//   //if it does not connect it starts an access point with the specified name
//   //here  "AutoConnectAP"
//   //and goes into a blocking loop awaiting configuration
//   if(!wifiManager.autoConnect()) {
//     Serial.println("failed to connect and hit timeout");
//     //reset and try again, or maybe put it to deep sleep
//     ESP.reset();
//     delay(1000);
//   }
//
//   //if you get here you have connected to the WiFi
//   Serial.println("WiFi connected.");
//
//
//
//   WiFi.hostByName(timerServerDNSName, timeServer);
//   Serial.print("Time server IP: ");
//   Serial.println(timeServer.toString());
//
//   Udp.begin(localPort);
//   Serial.print("UDP local port: ");
//   Serial.println(Udp.localPort());
//
//   time_t t = getNtpTime();
//   if (t != 0) {
//     Serial.println(t, DEC);
//
//     rtc.adjust(DateTime(t));
//     DateTime now = rtc.now();
//
//     Serial.println(now.unixtime(), DEC);
//
//     Serial.println("Board time:");
//     Serial.print(now.year(), DEC);
//     Serial.print('/');
//     Serial.print(now.month(), DEC);
//     Serial.print('/');
//     Serial.print(now.day(), DEC);
//     Serial.print(" ");
//     Serial.print(now.hour(), DEC);
//     Serial.print(':');
//     Serial.print(now.minute(), DEC);
//     Serial.print(':');
//     Serial.print(now.second(), DEC);
//     Serial.println();
//   }
//
//   printTemperature();
// }
//
//
//
// @endif
