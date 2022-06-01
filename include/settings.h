#ifndef __SETTINGS_H__
#define __SETTINGS_H__

class Settings {
public:
  Settings();
  bool getUseDialect();
  bool getUseThreeQuater();
  bool getUseQuaterPast();
  int getBrightness();
  void setBrightness(int brightness);
  void setUseDialect(bool useDialect);
  String getJsonString();
  sint8_t getUtcHourOffset();
  void setUtcHourOffset(sint8_t offset);
  u_int16_t getMainColor();
  void setMainColor(u_int8_t r, u_int8_t g, u_int8_t b);

private:
  bool m_useQuaterPast;
  bool m_useThreeQuater;
  bool m_useDialect;
  int m_brightness;
  u_int16_t m_mainColor;
  sint8_t m_utcHourOffset;
};

#endif
