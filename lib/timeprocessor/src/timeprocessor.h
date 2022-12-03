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
  F(ClockStr::Zero, "NULL", 0)                                                 \
  F(ClockStr::One, "EINS", 1)                                                  \
  F(ClockStr::Two, "ZWEI", 2)                                                  \
  F(ClockStr::Three, "DREI", 3)                                                \
  F(ClockStr::Four, "VIER", 4)                                                 \
  F(ClockStr::FiveHour, "FÜNF", 5)                                             \
  F(ClockStr::Six, "SECHS", 6)                                                 \
  F(ClockStr::Seven, "SIEBEN", 7)                                              \
  F(ClockStr::Eight, "ACHT", 8)                                                \
  F(ClockStr::Nine, "NEUN", 9)                                                 \
  F(ClockStr::TenHour, "ZEHN", 10)                                             \
  F(ClockStr::Eleven, "ELF", 11)                                               \
  F(ClockStr::Twelve, "ZWÖLF", 12)                                             \
  F(ClockStr::It, "ES", 25)                                                    \
  F(ClockStr::Is, "IST", 26)                                                   \
  F(ClockStr::Clock, "UHR", 27)                                                \
  F(ClockStr::Before, "REL_VOR", 28)                                           \
  F(ClockStr::After, "REL_NACH", 29)                                           \
  F(ClockStr::Quater, "REL_VIERTEL", 30)                                       \
  F(ClockStr::ThreeQuater, "REL_DREIVIERTEL", 31)                              \
  F(ClockStr::Half, "REL_HALB", 32)                                            \
  F(ClockStr::OneEven, "EIN", 33)                                              \
  F(ClockStr::Five, "REL_FÜNF", 34)                                            \
  F(ClockStr::Ten, "REL_ZEHN", 35)                                             \
  F(ClockStr::None, "UNBEKANNT", 36)

#define return_enum(key, val, hour)                                            \
  case hour:                                                                   \
    return key;

#define get_json_key(key, val, hour)                                           \
  case key:                                                                    \
    wordKey = val;                                                             \
    break;

#define get_enum_from_num(num)                                                 \
  switch (num) { time_table(return_enum) default : return ClockStr::None; }

#define json_key_from_state(state)                                             \
  switch (state) {                                                             \
    time_table(get_json_key);                                                  \
  default:                                                                     \
    wordKey = "UNBEKANNT";                                                     \
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
  void setQuaterPast(bool useQuaterPast);
  void setThreeQuater(bool useThreeQuater);
  void setOffsetLowSecs(int offsetLowSecs);
  void setOffsetHighSecs(int offsetHighSecs);
  void setDialect(bool useDialect);
  bool update(int hour, int minute, int seconds);
  bool update();
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
  bool checkLowBorder(int seconds, int target);
  bool checkHighBorder(int seconds, int target);
};

#endif
