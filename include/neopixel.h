#ifndef __NEOPIXEL_H__
#define __NEOPIXEL_H__

#include "color.h"
#include "pixel.h"
#include <Adafruit_NeoPixel.h>

class NeoPixel : Adafruit_NeoPixel {
public:
  NeoPixel(uint16_t n, int16_t pin = 6,
           neoPixelType type = NEO_GRB + NEO_KHZ800);
  ~NeoPixel();
  void setPixelType(uint16_t n, PixelType type);
  void setBrightness(double brighness, PixelType type);
  void setBrightness(double brighness);
  void setColor(COLOR_RGB color, PixelType type);
  void setColor(uint16_t n, COLOR_RGB color);
  void interpolate(uint8_t step);
  void show();
  void begin();
  PixelType getPixelType(uint16_t n);
  bool isBackgroundPixel(uint16_t n);
  bool isTimePixel(uint16_t n);
  bool isCustomPixel(uint16_t n);
  COLOR_RGB getTargetColor(uint16_t n);
  void setPixelColor(uint16_t n, uint32_t c);
  void update();

private:
  void setupPixelProps();
  Pixel *pixelProps;
};

#endif
