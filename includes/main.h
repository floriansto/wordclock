
enum class ClockStr
{
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

typedef struct _timestack
{
  bool useDialect;
  ClockStr state;
} TIMESTACK;

#define time_table(F)                                        \
  F(ClockStr::Zero, 0, "NULL", "NULL")                       \
  F(ClockStr::One, 1, "EINS", "EINSE")                       \
  F(ClockStr::Two, 2, "ZWEI", "ZWEIE")                       \
  F(ClockStr::Three, 3, "DREI", "DRII")                      \
  F(ClockStr::Four, 4, "VIER", "VIERE")                      \
  F(ClockStr::Five, 5, "FÜNF", "FÜNFE")                      \
  F(ClockStr::Six, 6, "SECHS", "SECHSE")                     \
  F(ClockStr::Seven, 7, "SIEBEN", "SIEBENE")                 \
  F(ClockStr::Eight, 8, "ACHT", "ACHDE")                     \
  F(ClockStr::Nine, 9, "NEUN", "NIINE")                      \
  F(ClockStr::Ten, 10, "ZEHN", "ZEHNE")                      \
  F(ClockStr::Eleven, 11, "ELF", "ELFE")                     \
  F(ClockStr::Twelve, 12, "ZWÖLF", "ZWÖLFE")                 \
  F(ClockStr::It, 13, "ES", "S")                             \
  F(ClockStr::Is, 14, "IST", "ISCH")                         \
  F(ClockStr::Clock, 15, "UHR", "UHR")                       \
  F(ClockStr::Before, 16, "VOR", "VOR")                      \
  F(ClockStr::After, 17, "NACH", "NACH")                     \
  F(ClockStr::Quater, 18, "VIERTEL", "VIERTEL")              \
  F(ClockStr::ThreeQuater, 19, "DREIVIERTEL", "DREIVIERTEL") \
  F(ClockStr::Half, 20, "HALB", "HALBER")                    \
  F(ClockStr::OneEven, 21, "EIN", "EINSE")                   \
  F(ClockStr::None, 22, "UNBEKANNT", "UNBEKANNT")

#define print_val_enum(val, num, val_human, val_dialect) \
  case val:                                              \
    Serial.print(val_human);                             \
    break;

#define return_enum(val, num, val_human, val_dialect) \
  case num:                                           \
    return val;

#define return_str_enum(val, num, val_human, val_dialect) \
  case val:                                               \
    return val_human;

#define return_str_dialect_enum(val, num, val_human, val_dialect) \
  case val:                                                       \
    return val_dialect;

#define print_state_enum(state)                                         \
  switch (state)                                                        \
  {                                                                     \
    time_table(print_val_enum) default : Serial.print("Unknown State"); \
    break;                                                              \
  }

#define get_enum_from_num(num)                               \
  switch (num)                                               \
  {                                                          \
    time_table(return_enum) default : return ClockStr::None; \
  }

#define get_str_from_enum(state)                              \
  switch (state)                                              \
  {                                                           \
    time_table(return_str_enum) default : return "UNBEKANNT"; \
  }

#define get_str_dialect_from_enum(state)                                      \
  switch (state)                                                              \
  {                                                                           \
    time_table(return_str_dialect_enum) default : return "UNBEKANNT_DIALECT"; \
  }
