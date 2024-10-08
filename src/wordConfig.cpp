#include "../include/wordConfig.h"

WordConfig::WordConfig() {
  this->enabled = false;
  this->timeEnabled = false;
  this->color.r = 0;
  this->color.g = 0;
  this->color.b = 0;
  this->when = ALWAYS;
  this->date.day = 0;
  this->date.month = 0;
  this->valid = false;
  for (uint8_t i = 0; i < MAX_LED_ENTRIES; ++i) {
    this->leds[i] = 0;
  }
}

boolean WordConfig::isWordConfigActive(uint16_t day, uint8_t month) {
  return this->enabled &&
         (this->when == ALWAYS ||
          (this->when == DATE && day == this->date.day && month == this->date.month));
}

void WordConfig::setEnabled(boolean enabled) {
  this->enabled = enabled;
}

void WordConfig::setShowTime(boolean showTime) {
  this->timeEnabled = showTime;
}

void WordConfig::setColor(COLOR_RGB color) {
  this->color = color;
}

void WordConfig::setWhen(LedConfigWhen when) {
  this->when = when;
}

void WordConfig::setDate(Date date) {
  this->date = date;
}

void WordConfig::setValid(boolean valid) {
  this->valid = valid;
}

void WordConfig::setLeds(uint32_t* leds, uint8_t length) {
  for (uint8_t i = 0; i < length; ++i) {
    this->leds[i] = leds[i];
  }
}

void WordConfig::serialize(JsonObject &json) {
  JsonArray leds;
  json["enable"] = this->enabled;
  json["useTime"] = this->timeEnabled;
  json["color"] = rgbToHex(this->color);
  json["when"] = when;
  json["date"]["day"] = this->date.day;
  json["date"]["month"] = this->date.month;
  leds = json.createNestedArray("leds");
  for (uint8_t i = 0; i < MAX_LED_ENTRIES; ++i) {
    leds.add(this->leds[i]);
  }
}

void WordConfig::deserialize(JsonDocument &json) {
  uint8_t idx{0};
  this->enabled = json["enable"];
  this->timeEnabled = json["useTime"];
  this->color = hexToRgb(json["color"]);
  this->when = (LedConfigWhen)json["when"];
  this->date.day = json["date"]["day"];
  this->date.month = json["date"]["month"];
  for (JsonVariant i : json["leds"].as<JsonArray>()) {
    this->leds[idx++] = i.as<uint32_t>();
  }
}

boolean WordConfig::isEnabled() { return this->enabled; }
boolean WordConfig::showTime() { return this->timeEnabled; }
boolean WordConfig::isValid() { return this->valid; }
uint32_t WordConfig::getLedsAt(uint8_t index) { return this->leds[index]; }
COLOR_RGB WordConfig::getColor() { return this->color; }
