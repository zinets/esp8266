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
  // adjustDateTime(1483228780);
  // 1475280000; // 01 Okt
  // 1480464000 // 30 Nov
  // 1481328000 // 10 Dec
  // 1483160400 // 31 Dec 5 am
  // 1483228680 // 2 minutes to NY
  // 1483228780 // 20 sec to NY

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
    "Su, ", "Mo, ", "Th, ", "We, ", "Th, ", "Fr, ", "Sa, ",
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

bool Clock::canShowNYRemainTime() {
  String t = getNYRemainingTime();
  Serial.println(t);
  Serial.println(t.length());
  bool res = t.length() > 0;
  if (res) {
    Serial.println("TRUE");
  } else {
    Serial.println("FALSE");
  }
  return res;
}

String Clock::getNYRemainingTime() {
  DateTime dt = rtc.now();
  int nextYear = dt.year() + 1;

  DateTime newYearDay = DateTime(nextYear, 1, 1, 0, 0, 0);
  TimeSpan d = newYearDay - dt;
  Serial.print ("remain ");
  Serial.println(d.days());
  Serial.println(d.hours());
  Serial.println(d.minutes());

  if (d.days() > 60) {
    return "";
  }

  if (d.days() > 1) {
    return String(d.days()) + " days";
  }
  if (d.hours() > 1) {
    return String(d.hours()) + " hours";
  }
  if (d.minutes() >= 1) {
    return String(d.minutes()) + " minutes";
  }
  return "Less then one minute";
}
