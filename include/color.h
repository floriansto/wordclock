#ifndef __COLOR_H__
#define __COLOR_H__

#include <Arduino.h>

struct RGB {
  int r, g, b;
};

struct LCH {
  double l, c, h;
};

struct XYZ {
  double x, y, z;
};

struct LAB {
  double l, a, b;
};

// Reference white after D65 and viewing angle of 10 degrees (CIE 1964)
#define xr 94.881
#define yr 100
#define zr 107.304

LCH rgb_to_lch(u_int32_t rgb);
u_int32_t lch_to_rgb(LCH lch);
u_int32_t lch_interp(LCH c1, LCH c2, double t);
u_int32_t rgb_interp(u_int32_t c1, u_int32_t c2, double t);

#endif
