#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "../include/color.h"

#define JSON_SETTINGS_SIZE 2048


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
  void setBrightness(int brightness);
  int getBrightness();
  void loadSettings();
  void saveSettings();
  void toJsonDoc(JsonObject &json);
  sint8_t getUtcHourOffset();
  void setUtcHourOffset(sint8_t offset);
  void setColor(String &rgbColor, const char *key);
  COLOR_RGB getBackgroundColor();
  void setWordConfig(String &wordConfig);
  COLOR_RGB getTimeColor();
  String getLangKey();
  LedWiring getLedWiring();

private:
  StaticJsonDocument<JSON_SETTINGS_SIZE> settings;
  LedWiring ledwiring;
};

#endif
