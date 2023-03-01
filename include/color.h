#ifndef __COLOR_H__
#define __COLOR_H__

//#include <Arduino.h>

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

LCH rgb_to_lch(RGB rgb);
RGB lch_to_rgb(LCH lch);
RGB lch_interp(LCH c1, LCH c2, double t);

#endif
