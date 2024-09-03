
#include "../include/pixel.h"
#include "../include/math.h"

Pixel::Pixel()
    : startColor(COLOR_RGB{0, 0, 0}), targetColor(COLOR_RGB{0, 0, 0}),
      brightness{0}, startBrightness{0}, targetBrightness{0} {}

void Pixel::update() {
  if (this->newTargetColor != this->targetColor) {
    this->startColor = interpolate(0, false);
    this->targetColor = this->newTargetColor;
    this->t = 0;
  }
  if (this->newTargetBrightness != this->targetBrightness) {
    this->startBrightness = this->brightness;
    this->targetBrightness = this->newTargetBrightness;
    this->t = 0;
  }
}

void Pixel::setTargetColor(COLOR_RGB color) { this->newTargetColor = color; }

void Pixel::setStartColor(COLOR_RGB color) { this->startColor = color; }

void Pixel::setBrightness(double brightness) {
  this->newTargetBrightness = brightness;
}

void Pixel::resetInterpolation() { this->t = 0; }

void Pixel::setPixelType(PixelType type) { this->type = type; }

PixelType Pixel::getType() { return this->type; }

COLOR_RGB Pixel::interpolate(uint8_t step, bool scale) {
  COLOR_RGB color;
  double t;

  if (this->t >= interpolationDuration) {
    this->brightness = this->targetBrightness;
    this->startColor = this->targetColor;
    this->startBrightness = this->targetBrightness;
    color = this->targetColor;
    if (scale) {
      color.r = (uint8_t)((double)color.r * brightness);
      color.g = (uint8_t)((double)color.g * brightness);
      color.b = (uint8_t)((double)color.b * brightness);
    }
    return color;
  }

  t = (double)this->t / (double)interpolationDuration;
  color = hexToRgb(
      rgb_interp(rgbToHex(this->startColor), rgbToHex(this->targetColor), t));
  this->brightness = lerp(this->startBrightness, this->targetBrightness, t);

  this->t = this->t + step < interpolationDuration ? this->t + step
                                                   : interpolationDuration;

  if (scale) {
    color.r = (uint8_t)((double)color.r * brightness);
    color.g = (uint8_t)((double)color.g * brightness);
    color.b = (uint8_t)((double)color.b * brightness);
  }
  return color;
}

bool Pixel::isBackgroundPixel() { return this->type == PixelType::Background; }

bool Pixel::isTimePixel() { return this->type == PixelType::Time; }
bool Pixel::isCustomPixel() { return this->type == PixelType::CustomWord; }

COLOR_RGB Pixel::getTargetColor() { return this->targetColor; }

COLOR_RGB Pixel::getStartColor() { return this->startColor; }

double Pixel::getBrightness() { return this->brightness; }
