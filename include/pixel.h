#ifndef __PIXEL_H__
#define __PIXEL_H__

#include "color.h"

const uint16_t interpolationDuration = 2000; /* ms */

enum class PixelType { Time, Background, CustomWord, Unknown };

class Pixel {
public:
  Pixel();
  void setTargetColor(COLOR_RGB color);
  void setStartColor(COLOR_RGB color);
  void setBrightness(double brightness);
  void resetInterpolation();
  COLOR_RGB interpolate(uint8_t step, bool scale);
  void setPixelType(PixelType type);
  PixelType getType();
  void update();
  bool isBackgroundPixel();
  bool isTimePixel();
  bool isCustomPixel();
  COLOR_RGB getTargetColor();
  COLOR_RGB getStartColor();
  double getBrightness();

private:
  COLOR_RGB newTargetColor;
  COLOR_RGB startColor;
  COLOR_RGB targetColor;
  double brightness;
  double startBrightness;
  double targetBrightness;
  double newTargetBrightness;
  PixelType type;
  uint16_t t;
};

#endif
