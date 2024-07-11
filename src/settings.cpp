#include "LittleFS.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#include "../include/hw_settings.h"
#include "../include/settings.h"

Settings::Settings(LedWiring ledWiring) {
  int mainColor[3]{252, 184, 33};
  int bgColor[3]{246, 245, 244};
  settings["brightnessSlider"] = 100;
  settings["switchDialect"] = false;
  settings["switchThreeQuater"] = true;
  settings["switchQuaterPast"] = true;
  settings["switchBackgroundColor"] = true;
  settings["utcTimeOffset"] = 1;
  JsonArray array = settings.createNestedArray("backgroundColor");
  copyArray(bgColor, array);
  array = settings.createNestedArray("timeColor");
  copyArray(mainColor, array);
  array = settings.createNestedArray("wordConfig");
  array[0]["enable"] = true;
  array[0]["words"] = "TIME";
  array[0]["when"] = "Always";
  JsonArray color = array[0].createNestedArray("color");
  copyArray(mainColor, color);
  this->ledwiring = ledWiring;
}

LedWiring Settings::getLedWiring() { return this->ledwiring; }

void Settings::setUseDialect(bool useDialect) {
  settings["switchDialect"] = useDialect;
}

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

bool Settings::getUseBackgroundColor() {
  return settings["switchBackgroundColor"];
}

int Settings::getBrightness() { return settings["brightnessSlider"]; }

void Settings::setBrightness(int brightness) {
  settings["brightnessSlider"] = brightness;
}

void Settings::setUtcHourOffset(sint8_t offset) {
  settings["utcTimeOffset"] = offset;
}

sint8_t Settings::getUtcHourOffset() { return settings["utcTimeOffset"]; }

JsonArray Settings::getWordConfig() { return settings["wordConfig"]; }

void Settings::setWordConfig(String &wordConfig) {
  StaticJsonDocument<1024> config;

  DeserializationError error = deserializeJson(config, wordConfig);
  if (error) {
    Serial.println("Failed to read settings.json using default configuration");
    Serial.println(error.f_str());
    return;
  }

  settings["wordConfig"].clear();
  settings.garbageCollect();

  for (JsonVariant value : config.as<JsonArray>()) {
    settings["wordConfig"].add(value);
  }

  Serial.println("Settings Memory: ");
  Serial.println(settings.memoryUsage());
}

COLOR_RGB getColor(JsonArray color) {
  return COLOR_RGB{color[0], color[1], color[2]};
}

void Settings::setColor(String &rgbColor, const char *key) {
  StaticJsonDocument<64> config;

  DeserializationError error = deserializeJson(config, rgbColor);
  if (error) {
    Serial.println("Failed to read settings.json using default configuration");
    Serial.println(error.f_str());
    return;
  }

  settings[key].set(config.as<JsonArray>());
}

COLOR_RGB Settings::getBackgroundColor() {
  if (this->getUseBackgroundColor()) {
    return getColor(settings["backgroundColor"]);
  }
  return COLOR_RGB{0, 0, 0};
}

COLOR_RGB Settings::getTimeColor() { return getColor(settings["timeColor"]); }

void Settings::toJsonDoc(JsonObject &json) {
  json = settings.as<JsonObject>();
  return;
}

void Settings::saveSettings() {
  File file = LittleFS.open("/settings.json", "w");

  if (!file) {
    Serial.println("settings.json not found!");
    return;
  }
  if (serializeJson(settings, file) == 0) {
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
  if (error) {
    Serial.println("Failed to read settings.json using default configuration");
    Serial.println(error.f_str());
    return;
  }
  Serial.println("Loaded settings");
}

String Settings::getLangKey() {
  if (this->getUseDialect()) {
    return "de-Dialect";
  }
  return "de-DE";
}
