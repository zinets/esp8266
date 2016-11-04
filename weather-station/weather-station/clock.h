#include "RTClib.h"

class Clock {
private:
  int localUtcOffset;
  RTC_DS1307 rtc;
public:
  Clock(int utcOffset);
  void adjustDateTime(time_t dateTime);

  String getTime();
  String getDate();
};
