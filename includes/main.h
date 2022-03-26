
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


#define time_table(F) \
  F(ClockStr::Zero,        0,  true,  "NULL", "NULL") \
  F(ClockStr::One,         1,  true,  "EINS", "EINSE") \
  F(ClockStr::Two,         2,  true,  "ZWEI", "ZWEIE") \
  F(ClockStr::Three,       3,  true,  "DREI", "DRII") \
  F(ClockStr::Four,        4,  true,  "VIER", "VIERE") \
  F(ClockStr::Five,        5,  true,  "FÜNF", "FÜNFE") \
  F(ClockStr::Six,         6,  true,  "SECHS", "SECHSE") \
  F(ClockStr::Seven,       7,  true,  "SIEBEN", "SIEBENE") \
  F(ClockStr::Eight,       8,  true,  "ACHT", "ACHDE") \
  F(ClockStr::Nine,        9,  true,  "NEUN", "NIINE") \
  F(ClockStr::Ten,         10, true,  "ZEHN", "ZEHNE") \
  F(ClockStr::Eleven,      11, true,  "ELF", "ELFE") \
  F(ClockStr::Twelve,      12, true,  "ZWÖLF", "ZWÖLFE") \
  F(ClockStr::It,          13, false, "ES", "S") \
  F(ClockStr::Is,          14, false, "IST", "ISCH") \
  F(ClockStr::Clock,       15, false, "UHR", "UHR") \
  F(ClockStr::Before,      16, false, "VOR", "VOR") \
  F(ClockStr::After,       17, false, "NACH", "NACH") \
  F(ClockStr::Quater,      18, false, "VIERTEL", "VIERTEL") \
  F(ClockStr::ThreeQuater, 19, false, "DREIVIERTEL", "DREIVIERTEL") \
  F(ClockStr::Half,        20, false, "HALB", "HALB") \
  F(ClockStr::OneEven,     21, false, "EIN", "EINSE") \
  F(ClockStr::None,        22, false, "UNBEKANNT", "UNBEKANNT")
  
#define print_val_enum(val, num, is_num, val_human, val_dialect) \
  case val: Serial.print(val_human); break;
  
#define return_enum(val, num, is_num, val_human, val_dialect) \
  case num: return val;
  
#define return_str_enum(val, num, is_num, val_human, val_dialect) \
  case val: return val_human;

#define return_str_dialect_enum(val, num, is_num, val_human, val_dialect) \
  case val: return val_dialect;

#define return_is_num(val, num, is_num, val_human, val_dialect) \
  case val: return is_num;

#define print_state_enum(state) \
  switch(state) { \
    time_table(print_val_enum) \
    default: Serial.print("Unknown State"); break; \
  }

#define get_enum_from_num(num) \
  switch(num) { \
    time_table(return_enum) \
    default: return ClockStr::None; \
  }

#define get_str_from_enum(state) \
  switch(state) { \
    time_table(return_str_enum) \
    default: return "UNBEKANNT"; \
  }

#define get_str_dialect_from_enum(state) \
  switch(state) { \
    time_table(return_str_dialect_enum) \
    default: return "UNBEKANNT"; \
  }

#define get_is_num_from_enum(state) \
  switch(state) { \
    time_table(return_is_num) \
    default: return false; \
  }
