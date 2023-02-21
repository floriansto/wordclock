#include "../include/led.h"

Led::Led() {
  this->interpol_param = 0;
  this->current = {.r = 0, .g = 0, .b = 0};
  this->start = {.r = 0, .g = 0, .b = 0};
  this->target = {.r = 0, .g = 0, .b = 0};
}

void Led::interpolateColors() {
  this->current.r =
      ((sin(this->interpol_param * PI / INTERPOLATION_TIME_MS - HALF_PI) + 1) /
           2 * (this->target.r - this->start.r) +
       this->start.r);
  this->current.g =
      ((sin(this->interpol_param * PI / INTERPOLATION_TIME_MS - HALF_PI) + 1) /
           2 * (this->target.g - this->start.g) +
       this->start.r);
  this->current.b =
      ((sin(this->interpol_param * PI / INTERPOLATION_TIME_MS - HALF_PI) + 1) /
           2 * (this->target.b - this->start.b) +
       this->start.b);
}

void Led::setTargetColor(COLOR color) {
  this->target = color;
  this->start = this->current;
  this->interpol_param = 0;
}

bool Led::raiseInterplateParam(u_int8_t value) {
  if (this->interpol_param + value > INTERPOLATION_TIME_MS) {
    return false;
  }
  this->interpol_param += value;
  return true;
}
