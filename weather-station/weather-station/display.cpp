#include "display.h"
#include <Time.h>

#define CLOCK_PIN   14
#define DATA_PIN    13
#define CS_PIN      10
#define DC_PIN      4
#define RST_PIN     5

#define LOCAL_UTC_OFFSET 2

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

Display::Display() {
  u8g2 = U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI(U8G2_R0, CLOCK_PIN, DATA_PIN, CS_PIN, DC_PIN, RST_PIN);

  u8g2.begin();
  u8g2.clearBuffer();
}

void Display::showTimeScreen(String currentTime, String currentDate) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_inb24_mr);

  const char *str = currentTime.c_str();
  int w = u8g2.getStrWidth(str);
  u8g2.drawStr((128 - w) / 2, 30, str);

  u8g2.setFont(u8g2_font_7x14_mf);
  str = currentDate.c_str();
  w = u8g2.getStrWidth(str);
  u8g2.drawStr((128 - w) / 2, 60, str);

  u8g2.sendBuffer();
}

void Display::setWeather(WeatherState newState) {
  currentWeatherState = newState;

  temperatures[0] = String(currentWeatherState.temperature) + " C";
  pressures[0] = String(currentWeatherState.pressure) + " mm";
}

void Display::showCurrentWeather() {
  u8g2.clearBuffer();

  int nums = sizeof(states) / sizeof(IconState);
  // Serial.println("nums = " + String(nums));
  for (int x = 0; x < nums; x++) {
    IconState state = states[x];
    if (state.condition == currentWeatherState.condition) {
      // Serial.println("state = " + state.condition);
      u8g2.drawXBMP(0, 0, 64, 64, state.xbm);
      break;
    }
  }

  u8g2.setFont(u8g2_font_unifont_t_cyrillic);
  u8g2.drawStr(50, 25, temperatures[0].c_str());
  u8g2.drawStr(50, 45, pressures[0].c_str());

  u8g2.sendBuffer();
}

void Display::showNYRemainTime(String remainingTime) {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 64, 64, xtree_bits);
  u8g2.setFont(u8g2_font_inr42_mn);
  const char *str = remainingTime.c_str();
  u8g2.drawStr(64 + (64 - u8g2.getStrWidth(str)) / 2, 53, str);
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

void Display::showConfigData(String apName) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);

  u8g2.drawStr(50, 15, "AP name:");
  u8g2.drawStr(50, 25, apName.c_str());

  u8g2.sendBuffer();
}
