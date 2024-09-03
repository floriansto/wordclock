#ifndef __MATH_H__
#define __MATH_H__

// Linearly interpolate between two double values
inline __attribute__((always_inline)) double lerp(double a, double b, double t) { return a * (1 - t) + b * t; }

#endif
