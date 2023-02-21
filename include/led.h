#ifndef __LED_H__
#define __LED_H__

#include <Arduino.h>

#define INTERPOLATION_TIME_MS 1500

typedef struct _color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} COLOR;

class Led {
public:
  Led();
  void interpolateColors();
  void setTargetColor(COLOR color);
  bool raiseInterplateParam(u_int8_t param);

private:
  u_int8_t interpol_param;
  COLOR current;
  COLOR start;
  COLOR target;
};

#endif
