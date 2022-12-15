#include <Arduino.h>
#include <ArduinoJson.h>
#include "LittleFS.h"

#include "../include/settings.h"
#include "../include/hw_settings.h"

Settings::Settings() {
  int mainColor[3]{0, 255, 0};
  int bgColor[3]{246, 245, 244};
  settings["brightnessSlider"] = 100;
  settings["switchDialect"] = false;
  settings["switchThreeQuater"] = true;
  settings["switchQuaterPast"] = true;
  settings["switchBackgroundColor"] = true;
  JsonArray array = settings.createNestedArray("mainColor");
  copyArray(mainColor, array);
  array = settings.createNestedArray("backgroundColor");
  copyArray(bgColor, array);
}

void Settings::setUseDialect(bool useDialect) { settings["switchDialect"] = useDialect; }

bool Settings::getUseDialect() { return settings["switchDialect"]; }

void Settings::setUseThreeQuater(bool useThreeQuater) {
  settings["switchThreeQuater"] = useThreeQuater;
}

bool Settings::getUseThreeQuater() { return settings["switchThreeQuater"]; }

void Settings::setUseQuaterPast(bool useQuaterPast) {
  settings["switchQuaterPast"] = useQuaterPast;
}

bool Settings::getUseQuaterPast() { return settings["switchQuaterPast"]; }

void Settings::setUseBackgroundColor(bool useBackgroundColor) {
  settings["switchBackgroundColor"] = useBackgroundColor;
}

bool Settings::getUseBackgroundColor() { return settings["switchBackgroundColor"]; }

int Settings::getBrightness() { return settings["brightnessSlider"]; }

void Settings::setBrightness(int brightness) { settings["brightnessSlider"] = brightness; }

void Settings::setUtcHourOffset(sint8_t offset) { m_utcHourOffset = offset; }

sint8_t Settings::getUtcHourOffset() { return m_utcHourOffset; }

void Settings::setMainColor(COLOR color) {
  settings["mainColor"][0] = color.r;
  settings["mainColor"][1] = color.g;
  settings["mainColor"][2] = color.b;
}

COLOR getColor(JsonArray color) {
  return COLOR{color[0], color[1], color[2]};
}

COLOR Settings::getMainColor() { return getColor(settings["mainColor"]); }

void Settings::setBackgroundColor(COLOR color) {
  settings["backgroundColor"][0] = color.r;
  settings["backgroundColor"][1] = color.g;
  settings["backgroundColor"][2] = color.b;
}

COLOR Settings::getBackgroundColor() { return getColor(settings["backgroundColor"]); }

void Settings::toJsonDoc(JsonDocument &json)
{
  json["brightnessSlider"] = settings["brightnessSlider"];
  json["switchDialect"] = (int)settings["switchDialect"];
  json["switchThreeQuater"] = (int)settings["switchThreeQuater"];
  json["switchQuaterPast"] = (int)settings["switchQuaterPast"];
  json["switchBackgroundColor"] = (int)settings["switchBackgroundColor"];
  json["mainColor"]["r"] = settings["mainColor"][0];
  json["mainColor"]["g"] = settings["mainColor"][1];
  json["mainColor"]["b"] = settings["mainColor"][2];
  json["backgroundColor"]["r"] = settings["backgroundColor"][0];
  json["backgroundColor"]["g"] = settings["backgroundColor"][1];
  json["backgroundColor"]["b"] = settings["backgroundColor"][2];
}

void Settings::saveSettings(JsonDocument &json) {
  File file = LittleFS.open("/settings.json", "w");

  if (!file) {
    Serial.println("settings.json not found!");
    return;
  }
  if (serializeJson(settings, file) == 0)
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
  DeserializationError error = deserializeJson(settings, file);
  if (error)
  {
    Serial.println("Failed to read settings.json using default configuration");
    Serial.println(error.f_str());
    return;
  }
  Serial.println("Loaded settings");
}
