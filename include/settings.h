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

private:
  bool m_useQuaterPast;
  bool m_useThreeQuater;
  bool m_useDialect;
  int m_brightness;
};

#endif
