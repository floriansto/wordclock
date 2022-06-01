#include <Arduino.h>
#include <Arduino_JSON.h>

#include "../include/settings.h"

Settings::Settings() {
  m_useDialect = false;
  m_useQuaterPast = true;
  m_useThreeQuater = true;
  m_brightness = 100;
  m_utcHourOffset = 1;
  m_mainColor = 0;
}

bool Settings::getUseDialect() { return m_useDialect; }

bool Settings::getUseThreeQuater() { return m_useThreeQuater; }

bool Settings::getUseQuaterPast() { return m_useQuaterPast; }

int Settings::getBrightness() { return m_brightness; }

void Settings::setBrightness(int brightness) { m_brightness = brightness; }

void Settings::setUseDialect(bool useDialect) { m_useDialect = useDialect; }

void Settings::setUtcHourOffset(sint8_t offset) { m_utcHourOffset = offset; }

sint8_t Settings::getUtcHourOffset() { return m_utcHourOffset; }

void Settings::setMainColor(u_int8_t r, u_int8_t g, u_int8_t b) {
  m_mainColor =
      ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

u_int16_t Settings::getMainColor() { return m_mainColor; }

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
