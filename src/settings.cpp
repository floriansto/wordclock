
#include "../include/settings.h"

Settings::Settings() {
  m_useDialect = false;
  m_useQuaterPast = true;
  m_useThreeQuater = true;
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

void Settings::setUseDialect(bool useDialect) {
  m_useDialect = useDialect;
}
