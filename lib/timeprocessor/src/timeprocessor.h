#ifndef __TIMEPROCESSOR_H__
#define __TIMEPROCESSOR_H__

#include <timestack.h>

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
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Eleven,
  Twelve,
  Before,
  After,
  Quater,
  ThreeQuater,
  Half
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
  F(ClockStr::Zero, 0, "NULL", 4, "NULL", 4)                                   \
  F(ClockStr::One, 1, "EINS", 4, "EINSE", 5)                                   \
  F(ClockStr::Two, 2, "ZWEI", 4, "ZWEIE", 5)                                   \
  F(ClockStr::Three, 3, "DREI", 4, "DRII", 4)                                  \
  F(ClockStr::Four, 4, "VIER", 4, "VIERE", 5)                                  \
  F(ClockStr::Five, 5, "FÜNF", 5, "FÜNFE", 6)                                  \
  F(ClockStr::Six, 6, "SECHS", 5, "SECHSE", 6)                                 \
  F(ClockStr::Seven, 7, "SIEBEN", 6, "SIEBENE", 8)                             \
  F(ClockStr::Eight, 8, "ACHT", 4, "ACHDE", 5)                                 \
  F(ClockStr::Nine, 9, "NEUN", 4, "NIINE", 5)                                  \
  F(ClockStr::Ten, 10, "ZEHN", 4, "ZEHNE", 5)                                  \
  F(ClockStr::Eleven, 11, "ELF", 3, "ELFE", 4)                                 \
  F(ClockStr::Twelve, 12, "ZWÖLF", 6, "ZWÖLFE", 7)                             \
  F(ClockStr::It, 13, "ES", 2, "S", 1)                                         \
  F(ClockStr::Is, 14, "IST", 3, "ISCH", 4)                                     \
  F(ClockStr::Clock, 15, "UHR", 3, "UHR", 3)                                   \
  F(ClockStr::Before, 16, "VOR", 3, "VOR", 3)                                  \
  F(ClockStr::After, 17, "NACH", 4, "NACH", 4)                                 \
  F(ClockStr::Quater, 18, "VIERTEL", 7, "VIERTEL", 7)                          \
  F(ClockStr::ThreeQuater, 19, "DREIVIERTEL", 11, "DREIVIERTEL", 11)           \
  F(ClockStr::Half, 20, "HALB", 4, "HALBER", 6)                                \
  F(ClockStr::OneEven, 21, "EIN", 3, "EINSE", 5)                               \
  F(ClockStr::None, 22, "UNBEKANNT", 9, "UNBEKANNT", 9)

#define return_enum(val, num, val_human, len, val_dialect, len_dialect)        \
  case num:                                                                    \
    return val;

#define assign_word(val, num, val_human, len, val_dialect, len_dialect)        \
  case val:                                                                    \
    *i = len;                                                                  \
    if (len <= maxWordLen) {                                                   \
      memcpy(word, val_human, sizeof(char) * len);                             \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
    break;

#define assign_word_dialect(val, num, val_human, len, val_dialect,             \
                            len_dialect)                                       \
  case val:                                                                    \
    *i = len_dialect;                                                          \
    if (len <= maxWordLen) {                                                   \
      memcpy(word, val_dialect, sizeof(char) * len_dialect);                   \
    } else {                                                                   \
      return false;                                                            \
    }                                                                          \
    break;

#define get_enum_from_num(num)                                                 \
  switch (num) { time_table(return_enum) default : return ClockStr::None; }

#define get_word(state, dialect)                                               \
  if (!dialect) {                                                              \
    switch (state) { time_table(assign_word); }                                \
  } else {                                                                     \
    switch (state) { time_table(assign_word_dialect); }                        \
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
  bool update(time_t epochTime);
  bool getWordTime(char *wordTime);

private:
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
  int getLowBorder(int seconds);
  int getHighBorder(int seconds);
  Times getTimeEnumFromSecs(int seconds);
  bool checkInterval(int seconds, int target);
  bool getDialect();
  bool getQuaterPast();
  bool getThreeQuater();
  Timestack *getStack();
  bool getTimeStack(Timestack *stack, time_t epochTime);
  bool calcWordTime();
};

#endif
