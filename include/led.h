#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>
#include "../include/color.h"

#define INTERPOLATION_TIME_MS 1500.0

class Led {
public:
  Led();
  void interpolateColors();
  void setTargetColor(RGB color);
  RGB getColor();
  RGB getTargetColor();
  bool raiseInterplateParam(u_int16_t param);

private:
  u_int16_t interpol_param;
  RGB current;
  RGB start;
  RGB target;
  bool interpol_active;
};

#endif
