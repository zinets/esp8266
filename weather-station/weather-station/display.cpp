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

/*
void Display::setTime(time_t dt) {
  #warning а вообще это все глупость; время берется у rtc у которого есть класс для даты; так что вот пусть он сформирует дату и просто передаст ее в сюда
  long t = (dt + 3600 * LOCAL_UTC_OFFSET + 86400) % 86400;
  int hours = ((t  % 86400L) / 3600) % 24;
  int minutes = ((t % 3600) / 60);
  int seconds = t % 60;

  // время с ведущим нулем
  currentTime = (hours < 10 ? "0" : "") + String (hours);
  // пульсирующий разделитель
  currentTime += (seconds % 2 == 0) ? ":" : " ";
  // минуты
  currentTime += (minutes < 10 ? "0" : "") + String (minutes);
}
*/

void Display::showTime() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);

  const char *str = currentTime.c_str();
  int w = u8g2.getStrWidth(str);
  u8g2.drawStr((128 - w) / 2, 20 , str);

  str = currentDate.c_str();
  w = u8g2.getStrWidth(str);
  u8g2.drawStr((128 - w) / 2, 40 , str);

  u8g2.sendBuffer();
}

void Display::showCurrentWeather() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 50, 50, wind_bits);
  u8g2.sendBuffer();
}
