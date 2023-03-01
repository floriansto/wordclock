#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>
#include "../include/color.h"

#define INTERPOLATION_TIME_MS 1500.0

class Led {
public:
  Led();
  void interpolateColors();
  void setTargetColor(COLOR color);
  COLOR getColor();
  COLOR getTargetColor();
  bool raiseInterplateParam(u_int16_t param);

private:
  u_int16_t interpol_param;
  COLOR current;
  COLOR start;
  COLOR target;
  bool interpol_active;
};

#endif
