#include "RTClib.h"

class Clock {
private:
  RTC_DS1307 rtc;
public:
  Clock();
  void adjustDateTime(time_t dateTime);
// getters
  String getTime();
  String getDate();
  bool canShowNYRemainTime();
  String getNYRemainingTime();
};
