#ifndef __MAIN_H__
#define __MAIN_H__

#include <timeprocessor.h>
#include <RTClib.h>

#define MAX_WORDS 25

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
  CYCLE_TIME_VIOLATION,
};

typedef enum _language { DE, DE_DIALECT, MAX_LANGUAGES } LANGUAGE;

#define language_list(F)                                                       \
  F(DE, "de-DE")                                                               \
  F(DE_DIALECT, "de-Dialect")

#define lang_key_from_value(key, value)                                        \
  if (strcmp(name, value) == 0) {                                              \
    return key;                                                                \
  }

#define map_lang_string_to_enum language_list(lang_key_from_value)

typedef struct _word_properties {
  String name;
  uint16_t startPixel;
  uint8_t length;
} WORD_PROPERTIES;
typedef struct _word {
  ClockStr type;
  WORD_PROPERTIES properties[MAX_LANGUAGES];
} WORD;

#endif
