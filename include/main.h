#ifndef __MAIN_H__
#define __MAIN_H__
typedef struct _time {
  u_int8_t hour;
  u_int8_t minute;
  u_int8_t seconds;
  u_int16_t year;
  u_int8_t month;
  u_int8_t day;
  bool valid;
} TIME;

typedef struct _rtc {
  RTC_DS3231 rtc;
  bool found;
  bool valid;
} RTC;

enum class Error {
  OK,
  NO_TIME,
  TIME_TO_WORD_CONVERSION,
  SUMMERTIME_ERROR,
  UPDATE_RTC_TIME_ERROR,
  TIMESTACK_GET_ELEM_FAILED,
};

#endif
