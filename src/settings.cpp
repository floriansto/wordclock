#include <Arduino.h>
#include <Arduino_JSON.h>

#include "../include/settings.h"

Settings::Settings() {
  m_useDialect = false;
  m_useQuaterPast = true;
  m_useThreeQuater = true;
  m_brightness = 100;
  m_utcHourOffset = 1;
}

bool Settings::getUseDialect() { return m_useDialect; }

bool Settings::getUseThreeQuater() { return m_useThreeQuater; }

bool Settings::getUseQuaterPast() { return m_useQuaterPast; }

int Settings::getBrightness() { return m_brightness; }

void Settings::setBrightness(int brightness) { m_brightness = brightness; }

void Settings::setUseDialect(bool useDialect) { m_useDialect = useDialect; }

void Settings::setUtcHourOffset(sint8_t offset) { m_utcHourOffset = offset; }

sint8_t Settings::getUtcHourOffset() { return m_utcHourOffset; }

String Settings::getJsonString() {
  JSONVar settingsValues;

  settingsValues["brightnessSlider"] = this->getBrightness();
  settingsValues["switchDialect"] = "false";
  if (this->getUseDialect() == true) {
    settingsValues["switchDialect"] = "true";
  }

  String jsonString = JSON.stringify(settingsValues);
  return jsonString;
}
