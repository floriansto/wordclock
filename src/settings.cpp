#include "LittleFS.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#include "../include/hw_settings.h"
#include "../include/settings.h"
#include "../include/wordConfig.h"
#include "../include/main.h"

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
  this->maxWordConfigs = 0;
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
  StaticJsonDocument<JSON_SIZE_WORD_CONFIG> config;
  uint32_t leds[MAX_LED_ENTRIES];

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
  while (i < MAX_LED_ENTRIES) {
    leds[i++] = 0;
  }

  this->wordConfig[maxWordConfigs].setEnabled(config["enable"].as<boolean>());
  this->wordConfig[maxWordConfigs].setShowTime(config["useTime"].as<boolean>());
  this->wordConfig[maxWordConfigs].setLeds(leds, MAX_LED_ENTRIES);
  this->wordConfig[maxWordConfigs].setColor(getColor(config["color"]));
  this->wordConfig[maxWordConfigs].setWhen((LedConfigWhen)config["when"].as<uint8_t>());
  this->wordConfig[maxWordConfigs].setDate(Date{config["date"]["day"].as<uint16_t>(), config["date"]["month"].as<uint8_t>()});
  this->wordConfig[maxWordConfigs].setValid(true);

  ++maxWordConfigs;
  Serial.println("Deserialize wordconfig: success");
}

COLOR_RGB stringToColor(String &rgbColor) {
  StaticJsonDocument<64> config;

  DeserializationError error = deserializeJson(config, rgbColor);
  if (error) {
    Serial.println("setColor: Failed to read color");
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

void Settings::serializeBasic(JsonObject &json) {
  json["brightness"] = this->brightness;
  json["useDialect"] = this->useDialect;
  json["useQuaterPast"] = this->useQuaterPast;
  json["useThreeQuater"] = this->useThreeQuater;
  json["useBackgroundColor"] = this->useBackgroundColor;
  json["utcTimeOffset"] = this->utcTimeOffset;
  json["timeColor"] = rgbToHex(this->timeColor);
  json["backgroundColor"] = rgbToHex(this->backgroundColor);
}

void Settings::deserializeBasic(JsonObject &json) {
  this->brightness = json["brightness"];
  this->useDialect = json["useDialect"];
  this->useQuaterPast = json["useQuaterPast"];
  this->useThreeQuater = json["useThreeQuater"];
  this->useBackgroundColor = json["useBackgroundColor"];
  this->utcTimeOffset = json["utcTimeOffset"];
  this->timeColor = hexToRgb(json["timeColor"]);
  this->backgroundColor = hexToRgb(json["backgroundColor"]);
}

void Settings::saveWordConfig() {
  LittleFS.remove("/customWordConfig.jsonl");
  File file = LittleFS.open("/customWordConfig.jsonl", "a");

  if (!file) {
    Serial.println("customWordConfig.jsonl not found!");
    return;
  }

  for (u_int8_t i = 0; i < this->maxWordConfigs; ++i) {
    StaticJsonDocument<JSON_SIZE_WORD_CONFIG> settings;
    JsonObject obj = settings.to<JsonObject>();
    this->wordConfig[i].serialize(obj);
    if (serializeJson(settings, file) == 0) {
      Serial.println("Failed to save custom word config");
      file.close();
      return;
    }
    file.print("\n");
  }

  file.close();
  Serial.println("Saved custom word config");
  return;
}

void Settings::saveSettings() {
  StaticJsonDocument<JSON_SIZE_SETTINGS> settings;
  JsonObject obj = settings.to<JsonObject>();
  File file = LittleFS.open("/settings.json", "w");

  if (!file) {
    Serial.println("settings.json not found!");
    return;
  }
  serializeBasic(obj);

  if (serializeJson(settings, file) == 0) {
    Serial.println("Failed to save settings");
    file.close();
    return;
  }

  file.close();
  Serial.println("Saved settings");

  return;
}

void Settings::loadWordConfig() {
  File file = LittleFS.open("/customWordConfig.jsonl", "r");
  if (!file) {
    Serial.println("customWordConfig.jsonl not found!");
    return;
  }

  this->maxWordConfigs = 0;
  while (true) {
    StaticJsonDocument<JSON_SIZE_WORD_CONFIG> settings;
    DeserializationError error = deserializeJson(settings, file);
    if (error) {
      Serial.println("Failed to read customWordConfig.jsonl using default configuration");
      Serial.println(error.f_str());
      break;
    }
    this->wordConfig[this->maxWordConfigs++].deserialize(settings);
  }
  Serial.println("Loaded custom word config");
}

void Settings::loadSettings() {
  StaticJsonDocument<JSON_SIZE_SETTINGS> settings;
  JsonObject obj = settings.to<JsonObject>();
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
  deserializeBasic(obj);
  Serial.println("Loaded settings");
}

LANGUAGE Settings::getLangKey() {
  if (this->getUseDialect()) {
    return DE_DIALECT;
  }
  return DE;
}
