#ifndef __MAIN_H__
#define __MAIN_H__

#include <timeprocessor.h>
#include <RTClib.h>

#define MAX_WORDS 25
#define MAX_WORD_LENGTH 15
#define MAX_WORDTIME_LENGTH 40

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

enum class MessageId {
  UNKNOWN,
  BRIGHTNESS,
  BACKGROUND_BRIGHTNESS,
  DIALECT,
  THREE_QUATER,
  QUATER_PAST,
  USE_BACKGROUND,
  BACKGROUND_COLOR,
  TIME_COLOR,
  UTC_TIME_OFFSET,
  WORDCONFIG,
  FINISHED_WORDCONFIG,
  CLEAR_WORDCONFIG,
  CONTINUE_SEND_WORDCONFIG,
  GET_VALUES,
  GET_TIME,
  CONTINUE_SEND_PREVIEW,
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
  char name[MAX_WORD_LENGTH];
  uint16_t startPixel;
  uint8_t length;
} WORD_PROPERTIES;
typedef struct _word {
  ClockStr type;
  WORD_PROPERTIES properties[MAX_LANGUAGES];
} WORD;

#endif
