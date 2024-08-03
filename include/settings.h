#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "../include/color.h"
#include "../include/wordConfig.h"
#include "../include/main.h"

#define MAX_WORD_CONFIGS 100
#define JSON_SIZE_WORD_CONFIG 384
#define JSON_SIZE_SETTINGS 256

enum class LedWiring {
  ZIGZAG,
  MATRIX,
};

class Settings {
public:
  Settings(LedWiring ledWiring);
  void setUseDialect(bool useDialect);
  bool getUseDialect();
  void setUseThreeQuater(bool useThreeQuater);
  bool getUseThreeQuater();
  void setUseQuaterPast(bool useQuaterPast);
  bool getUseQuaterPast();
  void setUseBackgroundColor(bool useBackgroundColor);
  bool getUseBackgroundColor();
  void setBrightness(uint8_t brightness);
  uint8_t getBrightness();
  void loadSettings();
  void saveSettings();
  void saveWordConfig();
  void loadWordConfig();
  void serializeBasic(JsonObject &json);
  void deserializeBasic(JsonObject &json);
  sint8_t getUtcHourOffset();
  void setUtcHourOffset(sint8_t offset);
  void setTimeColor(String &rgbColor);
  void setBackgroundColor(String &rgbColor);
  COLOR_RGB getBackgroundColor();
  void setWordConfig(String &wordConfig);
  WordConfig* getWordConfig();
  COLOR_RGB getTimeColor();
  LANGUAGE getLangKey();
  LedWiring getLedWiring();
  void clearWordConfig();
  uint8_t getMaxWordConfigs();

private:
  uint8_t brightness;
  bool useDialect;
  bool useThreeQuater;
  bool useQuaterPast;
  bool useBackgroundColor;
  COLOR_RGB backgroundColor;
  COLOR_RGB timeColor;
  uint8_t utcTimeOffset;
  LedWiring ledwiring;
  WordConfig wordConfig[MAX_WORD_CONFIGS];
  uint8_t maxWordConfigs;
};

#endif
