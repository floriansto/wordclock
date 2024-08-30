#include "../include/neopixel.h"

NeoPixel::NeoPixel(uint16_t n, int16_t pin, neoPixelType type)
    : Adafruit_NeoPixel{n, pin, type} {
  Adafruit_NeoPixel::setBrightness(255);
  this->setupPixelProps();
}

void NeoPixel::setupPixelProps() {
  uint32_t pixelPropsSize = this->numLEDs * sizeof(Pixel);
  free(this->pixelProps);
  this->pixelProps = (Pixel *)malloc(pixelPropsSize);
  if (this->pixelProps) {
    for (uint16_t i = 0; i < this->numLEDs; ++i) {
      this->pixelProps[i] = Pixel();
    }
  } else {
    numLEDs = 0;
    this->pixelProps = nullptr;
  }
}

void NeoPixel::setPixelType(uint16_t n, PixelType type) {
  if (n < this->numLEDs) {
    this->pixelProps[n].setPixelType(type);
  }
}

PixelType NeoPixel::getPixelType(uint16_t n) {
  if (n < this->numLEDs) {
    return this->pixelProps[n].getType();
  }
  return PixelType::Unknown;
}

void NeoPixel::setBrightness(double pixelBrightness, PixelType type) {
  for (uint8_t i = 0; i < this->numLEDs; ++i) {
    if (this->pixelProps[i].getType() == type) {
      this->pixelProps[i].setBrightness(pixelBrightness);
    }
  }
}

void NeoPixel::setBrightness(double pixelBrightness) {
  for (uint8_t i = 0; i < this->numLEDs; ++i) {
    this->pixelProps[i].setBrightness(pixelBrightness);
  }
}

void NeoPixel::setColor(COLOR_RGB color, PixelType type) {
  for (uint8_t i = 0; i < this->numLEDs; ++i) {
    if (this->pixelProps[i].getType() == type) {
      this->pixelProps[i].setTargetColor(color);
    }
  }
}

void NeoPixel::setColor(uint16_t led, COLOR_RGB color) {
  if (led < this->numLEDs) {
    this->pixelProps[led].setTargetColor(color);
  }
}

void NeoPixel::interpolate(uint8_t step) {
  COLOR_RGB color;
  double brightness;
  for (uint8_t i = 0; i < this->numLEDs; ++i) {
    color = this->pixelProps[i].interpolate(step);
    brightness = this->pixelProps[i].getBrightness();
    color.r = (uint8_t)((double)color.r * brightness);
    color.g = (uint8_t)((double)color.g * brightness);
    color.b = (uint8_t)((double)color.b * brightness);
    this->setPixelColor(i, rgbToHex(color));
  }
}

NeoPixel::~NeoPixel() { free(this->pixelProps); }

void NeoPixel::show() { Adafruit_NeoPixel::show(); }

void NeoPixel::update() {
  if (pixelProps != nullptr) {
  }
  for (uint8_t i = 0; i < this->numLEDs; ++i) {
    this->pixelProps[i].update();
  }
}

void NeoPixel::begin() { Adafruit_NeoPixel::begin(); }

bool NeoPixel::isBackgroundPixel(uint16_t n) {
  if (n < this->numLEDs) {
    return this->pixelProps[n].isBackgroundPixel();
  }
  return false;
}

bool NeoPixel::isTimePixel(uint16_t n) {
  if (n < this->numLEDs) {
    return this->pixelProps[n].isTimePixel();
  }
  return false;
}

bool NeoPixel::isCustomPixel(uint16_t n) {
  if (n < this->numLEDs) {
    return this->pixelProps[n].isCustomPixel();
  }
  return false;
}

COLOR_RGB NeoPixel::getTargetColor(uint16_t n) {
  if (n < this->numLEDs) {
    return this->pixelProps[n].getTargetColor();
  }
  return COLOR_RGB{0, 0, 0};
}

void NeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  Adafruit_NeoPixel::setPixelColor(n, c);
}
