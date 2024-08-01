#include "../include/wordConfig.h"

WordConfig::WordConfig() {}

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

boolean WordConfig::isEnabled() { return this->enabled; }
boolean WordConfig::showTime() { return this->timeEnabled; }
boolean WordConfig::isValid() { return this->valid; }
uint32_t WordConfig::getLedsAt(uint8_t index) { return this->leds[index]; }
COLOR_RGB WordConfig::getColor() { return this->color; }
