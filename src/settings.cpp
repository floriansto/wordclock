
#include "../include/settings.h"

Settings::Settings() {
  m_useDialect = false;
  m_useQuaterPast = true;
  m_useThreeQuater = true;
  m_brightness = 100;
}

bool Settings::getUseDialect() {
  return m_useDialect;
}

bool Settings::getUseThreeQuater() {
  return m_useThreeQuater;
}

bool Settings::getUseQuaterPast() {
  return m_useQuaterPast;
}

int Settings::getBrightness() {
  return m_brightness;
}

void Settings::setBrightness(int brightness) {
  m_brightness = brightness;
}

void Settings::setUseDialect(bool useDialect) {
  m_useDialect = useDialect;
}
