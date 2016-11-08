#include "display.h"
#include <Time.h>

#define CLOCK_PIN   14
#define DATA_PIN    13
#define CS_PIN      10
#define DC_PIN      4
#define RST_PIN     5

#define LOCAL_UTC_OFFSET 2

Display::Display() {
  u8g2 = U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI(U8G2_R0, CLOCK_PIN, DATA_PIN, CS_PIN, DC_PIN, RST_PIN);

  u8g2.begin();
  u8g2.clearBuffer();
}

void Display::showStartupScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(5, 25, "Starting...");
  u8g2.sendBuffer();
}

void Display::showTimeScreen(String currentTime, String currentDate) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_inb21_mf);

  const char *str = currentTime.c_str();
  int w = u8g2.getStrWidth(str);
  u8g2.drawStr((128 - w) / 2, 25, str);

  u8g2.setFont(u8g2_font_7x14_mf);
  str = currentDate.c_str();
  w = u8g2.getStrWidth(str);
  u8g2.drawStr((128 - w) / 2, 50, str);

  u8g2.sendBuffer();
}

void Display::showCurrentWeather() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 50, 50, wind_bits);
  u8g2.sendBuffer();
}

void Display::showNYRemainTime(String remainingTime) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(5, 25, remainingTime.c_str());
  u8g2.sendBuffer();
}

void Display::showIndoorData(String currentTemperature, String currentPressure) {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 2, temperature_width, temperature_height, temperature_bits);

  u8g2.setFont(u8g2_font_unifont_t_cyrillic);
  u8g2.drawStr(50, 25, currentTemperature.c_str());
  u8g2.drawStr(50, 45, currentPressure.c_str());

  u8g2.sendBuffer();
}

void Display::showWeatherData(String icon, String temperature, String pressure) {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 2, 50, 50, clear_night_bits);

  u8g2.setFont(u8g2_font_unifont_t_cyrillic);
  u8g2.drawStr(50, 25, temperature.c_str());
  u8g2.drawStr(50, 45, pressure.c_str());

  u8g2.sendBuffer();
}

void Display::showConfigData(String apName) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);

  u8g2.drawStr(50, 15, "AP name:");
  u8g2.drawStr(50, 25, apName.c_str());

  u8g2.sendBuffer();
}
