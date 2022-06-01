#ifndef __HW_SETTINGS_H__
#define __HW_SETTINGS_H__

#define PIN D6
#define COL_PIXELS 14
#define ROW_PIXELS 14
#define NUMPIXELS (COL_PIXELS * ROW_PIXELS)
#define INITIAL_BRIGHTNESS 100

const double max_current_ma = 400.0;
const double allowed_current_per_color_ma =
    (max_current_ma / (double)NUMPIXELS / 4.0);
const double max_current_per_color_ma = 11.0;
const u_int16_t offsetLowSecs = 150;
const u_int16_t offsetHighSecs = 300 - offsetLowSecs;

#endif
