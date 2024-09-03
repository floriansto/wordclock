#ifndef __PIXEL_H__
#define __PIXEL_H__

#include "color.h"

const uint16_t interpolationDuration = 400; /* ms */

enum class PixelType { Time, Background, CustomWord, Unknown };

class Pixel {
public:
  Pixel(double brightnessScaling);
  void setTargetColor(COLOR_RGB color);
  void setStartColor(COLOR_RGB color);
  void setBrightness(double brightness);
  void resetInterpolation();
  void interpolate(uint8_t step);
  COLOR_RGB getColor(bool scale);
  void setPixelType(PixelType type);
  PixelType getType();
  bool isBackgroundPixel();
  bool isTimePixel();
  bool isCustomPixel();
  COLOR_RGB getTargetColor();
  COLOR_RGB getStartColor();
  double getBrightness();
  void setBrightnessScaling(double scale);

private:
  COLOR_RGB newTargetColor;
  COLOR_RGB startColor;
  COLOR_RGB targetColor;
  COLOR_RGB color;
  double brightness;
  double startBrightness;
  double targetBrightness;
  double brightnessScaling;
  PixelType type;
  uint16_t t;
};

#endif
