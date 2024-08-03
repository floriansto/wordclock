#ifndef __WORDCONFIG_H__
#define __WORDCONFIG_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../include/hw_settings.h"
#include "../include/color.h"

#define MAX_LED_ENTRIES (NUMPIXELS / 32 + 1)
enum LedConfigWhen { ALWAYS, DATE, UNDEFINED };

struct Date {
  uint16_t day;
  uint8_t month;
};

class WordConfig {
public:
  WordConfig();
  boolean isWordConfigActive(uint16_t day, uint8_t month);
  void setEnabled(boolean enabled);
  void setShowTime(boolean showTime);
  void setColor(COLOR_RGB color);
  void setWhen(LedConfigWhen when);
  void setDate(Date date);
  void setValid(boolean valid);
  void setLeds(uint32_t* leds, uint8_t length);
  void serialize(JsonObject &json);
  void deserialize(JsonVariant &json);
  boolean isEnabled();
  boolean isValid();
  boolean showTime();
  uint32_t getLedsAt(uint8_t index);
  COLOR_RGB getColor();

private:
  uint32_t leds[MAX_LED_ENTRIES];
  boolean enabled;
  boolean timeEnabled;
  COLOR_RGB color;
  LedConfigWhen when;
  Date date;
  boolean valid;
};

#endif
