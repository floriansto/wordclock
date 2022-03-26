#ifndef __TIMEPROCESSOR_H__
#define __TIMEPROCESSOR_H__

#define WORDSTACK_SIZE 7

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

typedef struct _timestack {
  int size;
  bool useDialect;
  ClockStr state;
} TIMESTACK;

#define time_table(F)                                                          \
  F(ClockStr::Zero, 0, "NULL", "NULL")                                         \
  F(ClockStr::One, 1, "EINS", "EINSE")                                         \
  F(ClockStr::Two, 2, "ZWEI", "ZWEIE")                                         \
  F(ClockStr::Three, 3, "DREI", "DRII")                                        \
  F(ClockStr::Four, 4, "VIER", "VIERE")                                        \
  F(ClockStr::Five, 5, "FÜNF", "FÜNFE")                                        \
  F(ClockStr::Six, 6, "SECHS", "SECHSE")                                       \
  F(ClockStr::Seven, 7, "SIEBEN", "SIEBENE")                                   \
  F(ClockStr::Eight, 8, "ACHT", "ACHDE")                                       \
  F(ClockStr::Nine, 9, "NEUN", "NIINE")                                        \
  F(ClockStr::Ten, 10, "ZEHN", "ZEHNE")                                        \
  F(ClockStr::Eleven, 11, "ELF", "ELFE")                                       \
  F(ClockStr::Twelve, 12, "ZWÖLF", "ZWÖLFE")                                   \
  F(ClockStr::It, 13, "ES", "S")                                               \
  F(ClockStr::Is, 14, "IST", "ISCH")                                           \
  F(ClockStr::Clock, 15, "UHR", "UHR")                                         \
  F(ClockStr::Before, 16, "VOR", "VOR")                                        \
  F(ClockStr::After, 17, "NACH", "NACH")                                       \
  F(ClockStr::Quater, 18, "VIERTEL", "VIERTEL")                                \
  F(ClockStr::ThreeQuater, 19, "DREIVIERTEL", "DREIVIERTEL")                   \
  F(ClockStr::Half, 20, "HALB", "HALBER")                                      \
  F(ClockStr::OneEven, 21, "EIN", "EINSE")                                     \
  F(ClockStr::None, 22, "UNBEKANNT", "UNBEKANNT")

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

#define times_fcn_decl(state, minutes)                                         \
  bool setTime##minutes(TIMESTACK *stack, int *i);

#define select_time_fcn(state, minutes)                                        \
  case minutes:                                                                \
    ret &= setTime##minutes(stack, &i);                                        \
    break;

#define return_enum(val, num, val_human, val_dialect)                          \
  case num:                                                                    \
    return val;

#define return_str_enum(val, num, val_human, val_dialect)                      \
  case val:                                                                    \
    return val_human;

#define return_str_dialect_enum(val, num, val_human, val_dialect)              \
  case val:                                                                    \
    return val_dialect;

#define get_enum_from_num(num)                                                 \
  switch (num) { time_table(return_enum) default : return ClockStr::None; }

#define get_str_from_enum(state)                                               \
  switch (state) { time_table(return_str_enum) default : return "UNBEKANNT"; }

#define get_str_dialect_from_enum(state)                                       \
  switch (state) {                                                             \
    time_table(return_str_dialect_enum) default : return "UNBEKANNT_DIALECT";  \
  }

#define call_times_fcn(minutes)                                                \
  switch (minutes) { time_fcn(select_time_fcn) default : break; }

class TimeProcessor {
public:
  TimeProcessor(bool useDialect, bool useQuaterPast, bool useThreeQuater,
                int offsetLowSecs, int offsetHighSecs)
      : m_useDialect{useDialect}, m_useQuaterPast{useQuaterPast},
        m_useThreeQuater{useThreeQuater}, m_offsetLowSecs{offsetLowSecs},
        m_offsetHighSecs{offsetHighSecs} {}
  void setQuaterPast(bool useQuaterPast);
  void setThreeQuater(bool useThreeQuater);
  void setOffsetLowSecs(int offsetLowSecs);
  void setOffsetHighSecs(int offsetHighSecs);
  void setDialect(bool useDialect);
  int getTimeStack(TIMESTACK *stack, time_t epochTime);
  String evalTime(time_t epochTime);

private:
  bool m_useDialect;
  bool m_useQuaterPast;
  bool m_useThreeQuater;
  int m_offsetLowSecs;
  int m_offsetHighSecs;
  time_fcn(times_fcn_decl);
  int getOffsetLowSecs();
  int getOffsetHighSecs();
  int getLowBorder(int seconds);
  int getHighBorder(int seconds);
  bool checkInterval(int seconds, int target);
  bool getDialect();
  bool getQuaterPast();
  bool getThreeQuater();
};

#endif
