#include "LittleFS.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#include "../include/hw_settings.h"
#include "../include/settings.h"
#include "../include/wordConfig.h"

Settings::Settings(LedWiring ledWiring) {
  this->timeColor = COLOR_RGB{252, 184, 33};
  this->backgroundColor = COLOR_RGB{55, 55, 55};
  this->brightness = 50;
  this->useDialect = true;
  this->useThreeQuater = true;
  this->useQuaterPast = true;
  this->useBackgroundColor = true;
  this->utcTimeOffset = 1;
  this->ledwiring = ledWiring;
}

LedWiring Settings::getLedWiring() { return this->ledwiring; }

void Settings::setUseDialect(bool useDialect) {
  this->useDialect = useDialect;
}

bool Settings::getUseDialect() { return this->useDialect; }

void Settings::setUseThreeQuater(bool useThreeQuater) {
  this->useThreeQuater = useThreeQuater;
}

bool Settings::getUseThreeQuater() { return this->useThreeQuater; }

void Settings::setUseQuaterPast(bool useQuaterPast) {
  this->useQuaterPast = useQuaterPast;
}

bool Settings::getUseQuaterPast() { return this->useQuaterPast; }

void Settings::setUseBackgroundColor(bool useBackgroundColor) {
  this->useBackgroundColor = useBackgroundColor;
}

bool Settings::getUseBackgroundColor() {
  return this->useBackgroundColor;
}

uint8_t Settings::getBrightness() { return this->brightness; }

void Settings::setBrightness(uint8_t brightness) {
  this->brightness = brightness;
}

void Settings::setUtcHourOffset(sint8_t offset) {
  this->utcTimeOffset = offset;
}

sint8_t Settings::getUtcHourOffset() { return this->utcTimeOffset; }

WordConfig* Settings::getWordConfig() { return this->wordConfig; }

void Settings::clearWordConfig() {
  memset(this->wordConfig, 0, sizeof(this->wordConfig));
  this->maxWordConfigs = 0;
}

COLOR_RGB getColor(JsonArray color) {
  return COLOR_RGB{color[0], color[1], color[2]};
}

uint8_t Settings::getMaxWordConfigs() {return this->maxWordConfigs; }

void Settings::setWordConfig(String &wordConfig) {
  StaticJsonDocument<1024> config;
  uint32_t leds[MAX_WORD_CONFIGS];

  if (maxWordConfigs >= MAX_WORD_CONFIGS) {
    return;
  }

  DeserializationError error = deserializeJson(config, wordConfig);
  if (error) {
    Serial.println("setWordConfig: Failed to read json");
    Serial.println(error.f_str());
    return;
  }

  uint8_t i = 0;
  for (JsonInteger j : config["leds"].as<JsonArray>()) {
    leds[i++] = j;
  }
  for (i; i < MAX_LED_ENTRIES; ++i) {
    leds[i] = 0;
  }

  this->wordConfig[maxWordConfigs].setEnabled(config["enable"].as<boolean>());
  this->wordConfig[maxWordConfigs].setShowTime(config["useTime"].as<boolean>());
  this->wordConfig[maxWordConfigs].setLeds(leds, MAX_LED_ENTRIES);
  this->wordConfig[maxWordConfigs].setColor(getColor(config["color"]));
  this->wordConfig[maxWordConfigs].setWhen((LedConfigWhen)config["when"].as<uint8_t>());
  this->wordConfig[maxWordConfigs].setDate(Date{config["date"]["day"].as<uint16_t>(), config["date"]["month"].as<uint8_t>()});
  this->wordConfig[maxWordConfigs].setValid(true);

  ++maxWordConfigs;
}

COLOR_RGB stringToColor(String &rgbColor) {
  StaticJsonDocument<64> config;

  DeserializationError error = deserializeJson(config, rgbColor);
  if (error) {
    Serial.println("setColor: Failed to read json");
    Serial.println(error.f_str());
    return COLOR_RGB{0, 0, 0};
  }

  return getColor(config.as<JsonArray>());
}

void Settings::setTimeColor(String &rgbColor) {
  this->timeColor = stringToColor(rgbColor);
}

void Settings::setBackgroundColor(String &rgbColor) {
  this->backgroundColor = stringToColor(rgbColor);
}

COLOR_RGB Settings::getBackgroundColor() {
  if (this->useBackgroundColor) {
    return this->backgroundColor;
  }
  return COLOR_RGB{0, 0, 0};
}

COLOR_RGB Settings::getTimeColor() { return this->timeColor; }

//void Settings::toJsonDoc(JsonObject &json) {
//  json = settings.as<JsonObject>();
//  return;
//}

void Settings::saveSettings() {
  StaticJsonDocument<2048> settings;
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
  StaticJsonDocument<2048> settings;
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
