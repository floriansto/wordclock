#ifndef __SETTINGS_H__
#define __SETTINGS_H__

typedef struct _color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} COLOR;

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
  String getJsonString();
  void fromJsonString(String settings);
  sint8_t getUtcHourOffset();
  void setUtcHourOffset(sint8_t offset);
  COLOR getMainColor();
  void setMainColor(COLOR color);
  COLOR getBackgroundColor();
  void setBackgroundColor(COLOR color);
  JSONVar getJsonObject();

private:
  bool m_useQuaterPast;
  bool m_useThreeQuater;
  bool m_useDialect;
  bool m_useBackgroundColor;
  int m_brightness;
  COLOR m_mainColor;
  COLOR m_backgroundColor;
  sint8_t m_utcHourOffset;
};

#endif
