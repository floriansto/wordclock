#include "../include/color.h"
#include <Arduino.h>
#include <cmath>

// Linearly interpolate between two double values
double lerp(double a, double b, double t) { return a * (1 - t) + b * t; }

// Interpolate between two colors in CIELCH color space
u_int32_t lch_interp(LCH c1, LCH c2, double t) {
  // Interpolate L* and C* values linearly
  double l = lerp(c1.l, c2.l, t);
  double c = lerp(c1.c, c2.c, t);

  // Interpolate hue values using the shortest arc on the hue circle
  double h1 = c1.h;
  double h2 = c2.h;
  if (abs(h2 - h1) > 180) {
    if (h2 > h1) {
      h1 += 360;
    } else {
      h2 += 360;
    }
  }
  double h = lerp(h1, h2, t);
  if (h > 360) {
    h -= 360;
  } else if (h < 0) {
    h += 360;
  }

  // Convert interpolated CIELCH color to RGB color
  LCH interpolated_color = {l, c, h};
  return lch_to_rgb(interpolated_color);
}

u_int32_t rgb_interp(u_int32_t c1, u_int32_t c2, double t) {

  u_int8_t r1 = c1 >> 16;
  u_int8_t r2 = c2 >> 16;
  u_int8_t g1 = (c1 & 0x00FF00) >> 8;
  u_int8_t g2 = (c2 & 0x00FF00) >> 8;
  u_int8_t b1 = c1 & 0x0000FF;
  u_int8_t b2 = c2 & 0x0000FF;

  double r, g, b;

  r = lerp((double)r1, (double)r2, t);
  g = lerp((double)g1, (double)g2, t);
  b = lerp((double)b1, (double)b2, t);

  return ((int) r) << 16 | ((int) g) << 8 | ((int) b);
}

u_int32_t rgbToHex(COLOR_RGB color) {
  return color.r << 16 | color.g << 8 | color.b;
}

COLOR_RGB hexToRgb(u_int32_t hex) {
  u_int8_t r = (hex >> 16) & 0xFF;
  u_int8_t g = (hex >> 8) & 0xFF;
  u_int8_t b = (hex >> 0) & 0xFF;
  return COLOR_RGB{r, g, b};
}
