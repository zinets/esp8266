#include "clock.h"

Clock::Clock(int utcOffset) {
  localUtcOffset = utcOffset;

  rtc = RTC_DS1307();
  if (!rtc.begin()) {
    Serial.println("Hardware problem with RTC_1307?");

    while (1);
  }
  if (!rtc.isrunning()) {
    rtc.adjust(0);
  }
}

void Clock::adjustDateTime(time_t dateTime) {
  rtc.adjust(DateTime(dateTime));
}

String Clock::getTime() {
  DateTime dt = rtc.now();
  uint8_t d = dt.hour();
  String time = (d < 10 ? "0" : "") + String (d);
  d = dt.second();
  time += (d % 2 == 0) ? ":" : " ";
  d = dt.minute();
  time += (d < 10 ? "0" : "") + String(d);

  return time;
}
