
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
  F(ClockStr::Zero,        0,  "NULL") \
  F(ClockStr::One,         1,  "EINS") \
  F(ClockStr::Two,         2,  "ZWEI") \
  F(ClockStr::Three,       3,  "DREI") \
  F(ClockStr::Four,        4,  "VIER") \
  F(ClockStr::Five,        5,  "FÜNF") \
  F(ClockStr::Six,         6,  "SECHS") \
  F(ClockStr::Seven,       7,  "SIEBEN") \
  F(ClockStr::Eight,       8,  "ACHT") \
  F(ClockStr::Nine,        9,  "NEUN") \
  F(ClockStr::Ten,         10, "ZEHN") \
  F(ClockStr::Eleven,      11, "ELF") \
  F(ClockStr::Twelve,      12, "ZWÖLF") \
  F(ClockStr::It,          13, "ES") \
  F(ClockStr::Is,          14, "IST") \
  F(ClockStr::Clock,       15, "UHR") \
  F(ClockStr::Before,      16, "VOR") \
  F(ClockStr::After,       17, "NACH") \
  F(ClockStr::Quater,      18, "VIERTEL") \
  F(ClockStr::ThreeQuater, 19, "DREIVIERTEL") \
  F(ClockStr::Half,        20, "HALB") \
  F(ClockStr::OneEven,     21, "EIN") \
  F(ClockStr::None,        22, "UNBEKANNT")
  
#define print_val_enum(val, num, val_human) \
  case val: Serial.print(val_human); break;
  
#define return_enum(val, num, val_human) \
  case num: return val;
  
#define return_str_enum(val, num, val_human) \
  case val: return val_human;

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
