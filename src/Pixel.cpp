
#include "../include/pixel.h"
#include "../include/math.h"

Pixel::Pixel(double brightnessScaling)
    : startColor(COLOR_RGB{0, 0, 0}), targetColor(COLOR_RGB{0, 0, 0}),
      brightness{0}, startBrightness{0}, targetBrightness{0},
      brightnessScaling{brightnessScaling} {}

void Pixel::setTargetColor(COLOR_RGB color) {
  if (color != this->targetColor) {
    this->startColor = this->color;
    this->targetColor = color;
    this->startBrightness = this->brightness;
    this->t = 0;
  }
}

void Pixel::setStartColor(COLOR_RGB color) { this->startColor = color; }

void Pixel::setBrightness(double brightness) {
  if (brightness != this->targetBrightness) {
    this->startBrightness = this->brightness;
    this->targetBrightness = brightness;
    this->startColor = this->color;
    this->t = 0;
  }
}

void Pixel::resetInterpolation() { this->t = 0; }

void Pixel::setPixelType(PixelType type) { this->type = type; }

PixelType Pixel::getType() { return this->type; }

void Pixel::interpolate(uint8_t step) {
  double t;

  t = (double)this->t / (double)interpolationDuration;
  this->color = hexToRgb(
      rgb_interp(rgbToHex(this->startColor), rgbToHex(this->targetColor), t));
  this->brightness = lerp(this->startBrightness, this->targetBrightness, t);

  this->t = this->t + step < interpolationDuration ? this->t + step
                                                   : interpolationDuration;
}

COLOR_RGB Pixel::getColor(bool scale) {
  COLOR_RGB color{this->color};
  if (scale) {
    color.r =
        (uint8_t)((double)color.r * this->brightness * this->brightnessScaling);
    color.g =
        (uint8_t)((double)color.g * this->brightness * this->brightnessScaling);
    color.b =
        (uint8_t)((double)color.b * this->brightness * this->brightnessScaling);
  }
  return color;
}

bool Pixel::isBackgroundPixel() { return this->type == PixelType::Background; }

bool Pixel::isTimePixel() { return this->type == PixelType::Time; }
bool Pixel::isCustomPixel() { return this->type == PixelType::CustomWord; }

COLOR_RGB Pixel::getTargetColor() { return this->targetColor; }

COLOR_RGB Pixel::getStartColor() { return this->startColor; }

double Pixel::getBrightness() {
  return this->brightness * this->brightnessScaling;
}

void Pixel::setBrightnessScaling(double scale) {
  this->brightnessScaling = scale;
}
