#include "../include/color.h"
#include <Arduino.h>
#include <cmath>

// Convert a color from CIE XYZ to CIELAB color space
LAB xyz_to_lab(XYZ color) {
  // Normalize the XYZ values relative to the reference white point
  double xn = color.x / xr;
  double yn = color.y / yr;
  double zn = color.z / zr;

  // Nonlinear transformation to Lab space
  double fx =
      (xn > 0.008856) ? pow(xn, 1.0 / 3.0) : (7.787 * xn) + (16.0 / 116.0);
  double fy =
      (yn > 0.008856) ? pow(yn, 1.0 / 3.0) : (7.787 * yn) + (16.0 / 116.0);
  double fz =
      (zn > 0.008856) ? pow(zn, 1.0 / 3.0) : (7.787 * zn) + (16.0 / 116.0);

  double l = (116.0 * fy) - 16.0;
  double a = 500.0 * (fx - fy);
  double b = 200.0 * (fy - fz);

  return {l, a, b};
}

LCH rgb_to_lch(RGB rgb) {
  // Convert RGB values from 0-255 range to 0-1 range
  double r = rgb.r / 255.0;
  double g = rgb.g / 255.0;
  double b = rgb.b / 255.0;

  // Convert sRGB values to linear RGB values
  if (r <= 0.04045) {
    r /= 12.92;
  } else {
    r = pow((r + 0.055) / 1.055, 2.4);
  }
  if (g <= 0.04045) {
    g /= 12.92;
  } else {
    g = pow((g + 0.055) / 1.055, 2.4);
  }
  if (b <= 0.04045) {
    b /= 12.92;
  } else {
    b = pow((b + 0.055) / 1.055, 2.4);
  }

  // Convert linear RGB values to XYZ values using the transformation matrix
  double x = r * 0.4124 + g * 0.3576 + b * 0.1805;
  double y = r * 0.2126 + g * 0.7152 + b * 0.0722;
  double z = r * 0.0193 + g * 0.1192 + b * 0.9505;

  LAB lab = xyz_to_lab(XYZ{x, y, z});

  // Convert XYZ values to L*, C*, and h* values
  double c = sqrt(pow(lab.a, 2.0) + pow(lab.b, 2.0));
  double h = atan2(lab.b, lab.a) * 180 / PI;
  if (h < 0) {
    h += 360;
  }

  return {lab.l, c, h};
}

XYZ lab_to_xyz(LAB color) {
  double fy = (color.l + 16) / 116.0;
  double fx = fy + (color.a / 500.0);
  double fz = fy - (color.b / 200.0);

  double x_tmp = (fx > 0.206893034 ? pow(fx, 3) : (fx - 16.0 / 116.0) / 7.787);
  double y_tmp = (fy > 0.206893034 ? pow(fy, 3) : (fy - 16.0 / 116.0) / 7.787);
  double z_tmp = (fz > 0.206893034 ? pow(fz, 3) : (fz - 16.0 / 116.0) / 7.787);

  return {xr * x_tmp, yr * y_tmp, zr * z_tmp};
}

RGB lch_to_rgb(LCH lch) {
  // Convert L*, C*, and h* values to XYZ values
  double l = lch.l;
  double c = lch.c;
  double h = lch.h;
  double a_s = c * cos(h * PI / 180);
  double b_s = c * sin(h * PI / 180);

  XYZ xyz = lab_to_xyz(LAB{l, a_s, b_s});

  double x = xyz.x;
  double y = xyz.y;
  double z = xyz.z;

  // Convert XYZ values to linear RGB values using the inverse transformation
  // matrix
  double r = x * 3.2406 + y * -1.5372 + z * -0.4986;
  double g = x * -0.9689 + y * 1.8758 + z * 0.0415;
  double b = x * 0.0557 + y * -0.2040 + z * 1.0570;

  r = r > 1.0 ? 1.0 : r;
  g = g > 1.0 ? 1.0 : g;
  b = b > 1.0 ? 1.0 : b;

  r = r < 0.0 ? 0.0 : r;
  g = g < 0.0 ? 0.0 : g;
  b = b < 0.0 ? 0.0 : b;

  // Convert linear RGB values to sRGB values
  if (r > 0.0031308) {
    r = 1.055 * pow(r, 1.0 / 2.4) - 0.055;
  } else {
    r = 12.92 * r;
  }
  if (g > 0.0031308) {
    g = 1.055 * pow(g, 1.0 / 2.4) - 0.055;
  } else {
    g = 12.92 * g;
  }
  if (b > 0.0031308) {
    b = 1.055 * pow(b, 1.0 / 2.4) - 0.055;
  } else {
    b = 12.92 * b;
  }

  // Convert sRGB values from 0-1 range to 0-255 range
  int red = static_cast<int>(r * 255 + 0.5);
  int green = static_cast<int>(g * 255 + 0.5);
  int blue = static_cast<int>(b * 255 + 0.5);

  return {red, green, blue};
}

// Linearly interpolate between two double values
double lerp(double a, double b, double t) { return a * (1 - t) + b * t; }

// Interpolate between two colors in CIELCH color space
RGB lch_interp(LCH c1, LCH c2, double t) {
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
