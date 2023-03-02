#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>
#include "../include/color.h"

#define INTERPOLATION_TIME_MS 1500.0

#if 0
class Led {
public:
  Led();
  void interpolateColors();
  void setTargetColor(COLOR_RGB color);
  COLOR_RGB getColor();
  COLOR_RGB getTargetColor();
  bool raiseInterplateParam(u_int16_t param);

private:
  u_int16_t interpol_param;
  COLOR_RGB current;
  COLOR_RGB start;
  COLOR_RGB target;
  bool interpol_active;
};
#endif

#endif
