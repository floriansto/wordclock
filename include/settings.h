#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "../include/color.h"

#define JSON_SETTINGS_SIZE 2048

class Settings {
public:
  Settings();
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
  COLOR getBackgroundColor();
  void setWordConfig(String &wordConfig);
  COLOR getTimeColor();
  JsonVariant getTimeColorJson();

private:
  StaticJsonDocument<JSON_SETTINGS_SIZE> settings;
};

#endif
