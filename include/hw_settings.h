#ifndef __HW_SETTINGS_H__
#define __HW_SETTINGS_H__

#define PIN D6
#define COL_PIXELS 14
#define ROW_PIXELS 14
#define NUMPIXELS (COL_PIXELS * ROW_PIXELS)

const double maxCurrentAll = 3000.0;
const double maxCurrentPerLed = 50;
const u_int8_t cycleTimeMs = 10;
const u_int16_t offsetLowSecs = 150;
const u_int16_t offsetHighSecs = 300 - offsetLowSecs;

#endif
