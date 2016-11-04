#include "clock.h"

Clock::Clock(/*int utcOffset*/) {
  localUtcOffset = 2; //utcOffset;

  rtc = RTC_DS1307();
  if (!rtc.begin()) {
    Serial.println("Hardware problem with RTC_1307?");

    while (1);
  }
  if (!rtc.isrunning()) {
    Serial.println("Reset rtc");

    adjustDateTime(1478287596);
  }
}

void Clock::adjustDateTime(time_t dateTime) {
  rtc.adjust(DateTime(dateTime));
}

String Clock::getTime() {
  DateTime dt = rtc.now();
  uint8_t d = dt.hour();
  String result = (d < 10 ? "0" : "") + String (d);
  d = dt.second();

  result += (d % 2 == 0) ? ":" : " ";
  d = dt.minute();
  result += (d < 10 ? "0" : "") + String(d);

  return result;
}

String Clock::getDate() {
  DateTime dt = rtc.now();

  static String days[] = {
    //"Пн.", "Вт.", "Ср.", "Чт.", "Пт.", "Сб.", "Вс."
    "Mo, ", "Th, ", "We, ", "Th, ", "Fr, ", "Sa, ", "Su, "
  };
  // янв., февр., апр., авг., сент., окт., нояб., дек. Такие названия месяцев, как март, май, июнь, июль, сокращений не имеют.
  static String months[] = {
    // " янв. ", " февр. ", " март ", " апр. ", " май ", " июнь ",
    // " июль ", " авг. ", " сент. ", " окт. ", " нояб. ", " дек. "
    " Jan ", " Feb ", " Mar ", " Apr ", " May ", " Jun ",
    " Jul ", " Aug ", " Sep ", " Okt ", " Nov ", " Dec "
  };

  String result = days[dt.dayOfTheWeek()] + String(dt.day()) + months[dt.month() - 1] + String(dt.year());
  return result;
}
