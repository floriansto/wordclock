#include <Arduino.h>
#include <Arduino_JSON.h>

#include "../include/settings.h"

Settings::Settings() {
  m_useDialect = false;
  m_useQuaterPast = true;
  m_useThreeQuater = true;
  m_brightness = 100;
  m_utcHourOffset = 1;
  m_mainColor = COLOR{0, 255, 0};
}

void Settings::setUseDialect(bool useDialect) { m_useDialect = useDialect; }

bool Settings::getUseDialect() { return m_useDialect; }

void Settings::setUseThreeQuater(bool useThreeQuater) {
  m_useThreeQuater = useThreeQuater;
}

bool Settings::getUseThreeQuater() { return m_useThreeQuater; }

void Settings::setUseQuaterPast(bool useQuaterPast) {
  m_useQuaterPast = useQuaterPast;
}

bool Settings::getUseQuaterPast() { return m_useQuaterPast; }

int Settings::getBrightness() { return m_brightness; }

void Settings::setBrightness(int brightness) { m_brightness = brightness; }

void Settings::setUtcHourOffset(sint8_t offset) { m_utcHourOffset = offset; }

sint8_t Settings::getUtcHourOffset() { return m_utcHourOffset; }

void Settings::setMainColor(COLOR color) { m_mainColor = color; }

COLOR Settings::getMainColor() { return m_mainColor; }

String Settings::getJsonString() {
  JSONVar settingsValues;

  settingsValues["brightnessSlider"] = this->getBrightness();
  settingsValues["switchDialect"] = (int)this->getUseDialect();
  settingsValues["switchThreeQuater"] = (int)this->getUseThreeQuater();
  settingsValues["switchQuaterPast"] = (int)this->getUseQuaterPast();
  settingsValues["mainColor"]["r"] = m_mainColor.r;
  settingsValues["mainColor"]["g"] = m_mainColor.g;
  settingsValues["mainColor"]["b"] = m_mainColor.b;

  String jsonString = JSON.stringify(settingsValues);
  return jsonString;
}

void Settings::fromJsonString(String settings) {
  JSONVar json = JSON.parse(settings);
  m_brightness = json["brightnessSlider"];
  m_useDialect = ((int)json["switchDialect"]) > 0;
  m_useThreeQuater = ((int)json["switchThreeQuater"]) > 0;
  m_useQuaterPast = ((int)json["switchQuaterPast"]) > 0;
  m_mainColor.r = (int)json["mainColor"]["r"];
  m_mainColor.g = (int)json["mainColor"]["g"];
  m_mainColor.b = (int)json["mainColor"]["b"];
}
