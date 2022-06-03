#ifndef __TIMEPROCESSOR_H__
#define __TIMEPROCESSOR_H__

#include "timestack.h"

enum class ClockStr {
  None,
  It,
  Is,
  Clock,
  Zero,
  One,
  OneEven,
  Two,
  Three,
  Four,
  FiveHour,
  Six,
  Seven,
  Eight,
  Nine,
  TenHour,
  Eleven,
  Twelve,
  Before,
  After,
  Quater,
  ThreeQuater,
  Half,
  Five,
  Ten,
};

enum class Times {
  Full,
  FiveAfter,
  TenAfter,
  QuaterPast,
  TwentyAfter,
  TwentyFiveAfter,
  Half,
  TwentyFiveBefore,
  TwentyBefore,
  QuaterBefore,
  TenBefore,
  FiveBefore
};

#define time_table(F)                                                          \
  F(ClockStr::Zero, 0, "NULL", 4, 0, 0, 0, 0)                                  \
  F(ClockStr::One, 1, "EINS", 4, 1, 2, 4, 1)                                   \
  F(ClockStr::Two, 2, "ZWEI", 4, 0, 6, 4, 1)                                   \
  F(ClockStr::Three, 3, "DREI", 4, 9, 6, 4, 1)                                 \
  F(ClockStr::Four, 4, "VIER", 4, 6, 2, 4, 1)                                  \
  F(ClockStr::FiveHour, 5, "FÜNF", 5, 1, 8, 4, 1)                              \
  F(ClockStr::Six, 6, "SECHS", 5, 8, 7, 5, 1)                                  \
  F(ClockStr::Seven, 7, "SIEBEN", 6, 7, 5, 6, 1)                               \
  F(ClockStr::Eight, 8, "ACHT", 4, 9, 1, 4, 1)                                 \
  F(ClockStr::Nine, 9, "NEUN", 4, 0, 1, 4, 1)                                  \
  F(ClockStr::TenHour, 10, "ZEHN", 4, 8, 8, 4, 1)                              \
  F(ClockStr::Eleven, 11, "ELF", 3, 0, 7, 3, 1)                                \
  F(ClockStr::Twelve, 12, "ZWÖLF", 6, 0, 5, 5, 1)                              \
  F(ClockStr::It, 13, "ES", 2, 0, 13, 2, 1)                                    \
  F(ClockStr::Is, 14, "IST", 3, 9, 13, 3, 1)                                   \
  F(ClockStr::Clock, 15, "UHR", 3, 5, 0, 3, 1)                                 \
  F(ClockStr::Before, 16, "VOR", 3, 6, 10, 3, 1)                               \
  F(ClockStr::After, 17, "NACH", 4, 1, 10, 4, 1)                               \
  F(ClockStr::Quater, 18, "VIERTEL", 7, 7, 12, 7, 1)                           \
  F(ClockStr::ThreeQuater, 19, "DREIVIERTEL", 11, 3, 12, 11, 1)                \
  F(ClockStr::Half, 20, "HALB", 4, 10, 10, 4, 1)                               \
  F(ClockStr::OneEven, 21, "EIN", 3, 1, 2, 3, 1)                               \
  F(ClockStr::Five, 22, "FÜNF", 5, 0, 11, 4, 1)                                \
  F(ClockStr::Ten, 23, "ZEHN", 4, 7, 11, 4, 1)                                 \
  F(ClockStr::None, 24, "UNBEKANNT", 9, 0, 0, 2, 2)

#define time_table_dialect(F)                                                  \
  F(ClockStr::Zero, 0, "NULL", 4, 0, 0, 0, 0)                                  \
  F(ClockStr::One, 1, "EINSE", 5, 1, 2, 5, 1)                                  \
  F(ClockStr::Two, 2, "ZWEIE", 5, 0, 6, 5, 1)                                  \
  F(ClockStr::Three, 3, "DRII", 4, 5, 6, 4, 1)                                 \
  F(ClockStr::Four, 4, "VIERE", 5, 6, 2, 5, 1)                                 \
  F(ClockStr::FiveHour, 5, "FÜNFE", 6, 1, 8, 5, 1)                             \
  F(ClockStr::Six, 6, "SECHSE", 6, 8, 7, 6, 1)                                 \
  F(ClockStr::Seven, 7, "SIEBENE", 7, 7, 5, 7, 1)                              \
  F(ClockStr::Eight, 8, "ACHDE", 5, 0, 0, 5, 1)                                \
  F(ClockStr::Nine, 9, "NIINE", 5, 3, 1, 5, 1)                                 \
  F(ClockStr::TenHour, 10, "ZEHNE", 5, 8, 8, 5, 1)                             \
  F(ClockStr::Eleven, 11, "ELFE", 4, 0, 7, 4, 1)                               \
  F(ClockStr::Twelve, 12, "ZWÖLFE", 7, 0, 5, 6, 1)                             \
  F(ClockStr::It, 13, "S", 1, 1, 13, 1, 1)                                     \
  F(ClockStr::Is, 14, "ISCH", 4, 5, 13, 4, 1)                                  \
  F(ClockStr::Clock, 15, "UHR", 3, 5, 0, 3, 1)                                 \
  F(ClockStr::Before, 16, "VOR", 3, 6, 10, 3, 1)                               \
  F(ClockStr::After, 17, "NACH", 4, 1, 10, 4, 1)                               \
  F(ClockStr::Quater, 18, "VIERTEL", 7, 7, 12, 7, 1)                           \
  F(ClockStr::ThreeQuater, 19, "DREIVIERTEL", 11, 3, 12, 11, 1)                \
  F(ClockStr::Half, 20, "HALB", 4, 10, 10, 4, 1)                               \
  F(ClockStr::OneEven, 21, "EINSE", 5, 1, 2, 5, 1)                             \
  F(ClockStr::Five, 22, "FÜNF", 5, 0, 11, 4, 1)                                \
  F(ClockStr::Ten, 23, "ZEHN", 4, 7, 11, 4, 1)                                 \
  F(ClockStr::None, 24, "UNBEKANNT", 9, 0, 0, 2, 2)

#define return_enum(val, num, val_human, len, x_pos, y_pos, x_len, y_len)      \
  case num:                                                                    \
    return val;

#define assign_word(val, num, val_human, len, x_pos, y_pos, x_len, y_len)      \
  case val:                                                                    \
    *i = len;                                                                  \
    if (len <= maxWordLen) {                                                   \
      memcpy(word, val_human, sizeof(char) * len);                             \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
    break;

#define assign_led_rectangle(val, num, val_human, len, x_pos, y_pos, x_len,    \
                             y_len)                                            \
  case val:                                                                    \
    buffer[0] = x_pos;                                                         \
    buffer[1] = y_pos;                                                         \
    buffer[2] = x_len;                                                         \
    buffer[3] = y_len;                                                         \
    break;

#define get_enum_from_num(num)                                                 \
  switch (num) { time_table(return_enum) default : return ClockStr::None; }

#define get_word(state, dialect)                                               \
  if (!dialect) {                                                              \
    switch (state) { time_table(assign_word); }                                \
  } else {                                                                     \
    switch (state) { time_table_dialect(assign_word); }                        \
  }

#define get_led_rectangle(state, dialect, buffer)                              \
  if (!dialect) {                                                              \
    switch (state) {                                                           \
      time_table(assign_led_rectangle);                                        \
    default:                                                                   \
      buffer[0] = 0;                                                           \
      buffer[1] = 0;                                                           \
      buffer[2] = 2;                                                           \
      buffer[3] = 2;                                                           \
    }                                                                          \
  } else {                                                                     \
    switch (state) {                                                           \
      time_table_dialect(assign_led_rectangle);                                \
    default:                                                                   \
      buffer[0] = 0;                                                           \
      buffer[1] = 0;                                                           \
      buffer[2] = 2;                                                           \
      buffer[3] = 2;                                                           \
    }                                                                          \
  }

#define time_fcn(F)                                                            \
  F(Times::Full, 0)                                                            \
  F(Times::FiveAfter, 5)                                                       \
  F(Times::TenAfter, 10)                                                       \
  F(Times::QuaterPast, 15)                                                     \
  F(Times::TwentyAfter, 20)                                                    \
  F(Times::TwentyFiveAfter, 25)                                                \
  F(Times::Half, 30)                                                           \
  F(Times::TwentyFiveBefore, 35)                                               \
  F(Times::TwentyBefore, 40)                                                   \
  F(Times::QuaterBefore, 45)                                                   \
  F(Times::TenBefore, 50)                                                      \
  F(Times::FiveBefore, 55)

#define times_fcn_decl(state, minutes) bool setTime##minutes(Timestack *stack);

#define select_time_fcn(state, minutes)                                        \
  case state:                                                                  \
    ret &= setTime##minutes(stack);                                            \
    break;

#define return_time_fcn_enum(state, num)                                       \
  if (checkInterval(seconds, num * 60)) {                                      \
    return state;                                                              \
  }

#define get_times_enum_from_num(num) time_fcn(return_time_fcn_enum);

#define call_times_fcn(state)                                                  \
  switch (state) { time_fcn(select_time_fcn) default : break; }

bool getWord(TIMESTACK *elem, char *word, int maxWordLen, int *i);

class TimeProcessor {
public:
  TimeProcessor(bool useDialect, bool useQuaterPast, bool useThreeQuater,
                int offsetLowSecs, int offsetHighSecs, int numLetters);
  ~TimeProcessor();
  void setQuaterPast(bool useQuaterPast);
  void setThreeQuater(bool useThreeQuater);
  void setOffsetLowSecs(int offsetLowSecs);
  void setOffsetHighSecs(int offsetHighSecs);
  void setDialect(bool useDialect);
  bool update(int hour, int minute, int seconds);
  bool update();
  bool getWordTime(char *wordTime);
  Timestack *getStack();

private:
  int m_hour;
  int m_minutes;
  int m_seconds;
  bool m_useDialect;
  bool m_useQuaterPast;
  bool m_useThreeQuater;
  int m_offsetLowSecs;
  int m_offsetHighSecs;
  int m_numLetters;
  char *m_wordtime;
  bool m_error;
  Timestack m_stack;

  time_fcn(times_fcn_decl);
  int getOffsetLowSecs();
  int getOffsetHighSecs();
  Times getTimeEnumFromSecs(int seconds);
  bool checkInterval(int seconds, int target);
  bool getDialect();
  bool getQuaterPast();
  bool getThreeQuater();
  bool getTimeStack(Timestack *stack);
  bool calcWordTime();
  bool checkLowBorder(int seconds, int target);
  bool checkHighBorder(int seconds, int target);
};

#endif
