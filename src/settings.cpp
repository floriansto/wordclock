#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"

#include "../include/settings.h"
#include "../include/hw_settings.h"

Settings::Settings() {
  m_useDialect = false;
  m_useQuaterPast = true;
  m_useThreeQuater = true;
  m_useBackgroundColor = false;
  m_brightness = 100;
  m_utcHourOffset = 1;
  m_mainColor = COLOR{0, 255, 0};
  m_backgroundColor = COLOR{255, 0, 0};
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

void Settings::setUseBackgroundColor(bool useBackgroundColor) {
  m_useBackgroundColor = useBackgroundColor;
}

bool Settings::getUseBackgroundColor() { return m_useBackgroundColor; }

int Settings::getBrightness() { return m_brightness; }

void Settings::setBrightness(int brightness) { m_brightness = brightness; }

void Settings::setUtcHourOffset(sint8_t offset) { m_utcHourOffset = offset; }

sint8_t Settings::getUtcHourOffset() { return m_utcHourOffset; }

void Settings::setMainColor(COLOR color) { m_mainColor = color; }

COLOR Settings::getMainColor() { return m_mainColor; }

void Settings::setBackgroundColor(COLOR color) { m_backgroundColor = color; }

COLOR Settings::getBackgroundColor() { return m_backgroundColor; }

void Settings::toJsonDoc(JsonDocument &json)
{
  json["brightnessSlider"] = this->getBrightness();
  json["switchDialect"] = (int)this->getUseDialect();
  json["switchThreeQuater"] = (int)this->getUseThreeQuater();
  json["switchQuaterPast"] = (int)this->getUseQuaterPast();
  json["switchBackgroundColor"] = (int)this->getUseBackgroundColor();
  json["mainColor"]["r"] = m_mainColor.r;
  json["mainColor"]["g"] = m_mainColor.g;
  json["mainColor"]["b"] = m_mainColor.b;
  json["backgroundColor"]["r"] = m_backgroundColor.r;
  json["backgroundColor"]["g"] = m_backgroundColor.g;
  json["backgroundColor"]["b"] = m_backgroundColor.b;
}

void Settings::saveSettings(JsonDocument &json) {
  this->toJsonDoc(json);

  File file = LittleFS.open("/settings.json", "w");
  if (!file) {
    Serial.println("settings.json not found!");
    return;
  }
  if (serializeJson(json, file) == 0)
  {
    Serial.println("Failed to save settings");
    file.close();
    return;
  }
  file.close();
  Serial.println("Saved settings");

  return;
}

void Settings::loadSettings() {
  File file = LittleFS.open("/settings.json", "r");
  if (!file) {
    Serial.println("settings.json not found!");
    return;
  }
  StaticJsonDocument<JSON_SETTINGS_SIZE> json;
  DeserializationError error = deserializeJson(json, file);
  if (error)
  {
    Serial.println("Failed to read settings.json using default configuration");
    Serial.println(error.f_str());
    return;
  }

  m_brightness = json["brightnessSlider"];
  m_useDialect = ((int)json["switchDialect"]) > 0;
  m_useThreeQuater = ((int)json["switchThreeQuater"]) > 0;
  m_useQuaterPast = ((int)json["switchQuaterPast"]) > 0;
  m_useBackgroundColor = ((int)json["switchBackgroundColor"]) > 0;
  m_mainColor.r = (int)json["mainColor"]["r"];
  m_mainColor.g = (int)json["mainColor"]["g"];
  m_mainColor.b = (int)json["mainColor"]["b"];
  m_backgroundColor.r = (int)json["backgroundColor"]["r"];
  m_backgroundColor.g = (int)json["backgroundColor"]["g"];
  m_backgroundColor.b = (int)json["backgroundColor"]["b"];
}
