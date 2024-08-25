#ifndef __COLOR_H__
#define __COLOR_H__

#include <Arduino.h>

struct COLOR_RGB {
  u_int8_t r, g, b;

  inline COLOR_RGB &operator=(const uint32_t hexColor)
      __attribute__((always_inline)) {
    r = (hexColor >> 16) & 0xFF;
    g = (hexColor >> 8) & 0xFF;
    b = (hexColor >> 0) & 0xFF;
    return *this;
  }
};

inline __attribute__((always_inline)) bool operator==(const COLOR_RGB &a,
                                                      const COLOR_RGB &b) {
  return (a.r == b.r) && (a.g == b.g) && (a.b == b.b);
}

inline __attribute__((always_inline)) bool operator!=(const COLOR_RGB &a,
                                                      const COLOR_RGB &b) {
  return !(a == b);
}

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
u_int32_t rgbToHex(COLOR_RGB color);
COLOR_RGB hexToRgb(u_int32_t hex);

#endif
