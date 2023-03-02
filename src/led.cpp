#include "../include/led.h"

#if 0
Led::Led() {
  this->interpol_param = 0;
  this->current = {.r = 0, .g = 0, .b = 0};
  this->start = {.r = 0, .g = 0, .b = 0};
  this->target = {.r = 0, .g = 0, .b = 0};
}

void Led::interpolateColors() {
  if (this->target.r == this->start.r && this->target.g == this->start.g &&
      this->target.b == this->start.b) {
    //Serial.println("No interpolation");
    return;
  }
  if (this->interpol_active == false) {
    return;
  }
  this->current.r = (u_int8_t)
      ((sin((float)this->interpol_param * PI / INTERPOLATION_TIME_MS - HALF_PI) + 1.0) /
           2.0 * (float)(this->target.r - this->start.r) +
       this->start.r);
  this->current.g = (u_int8_t)
      ((sin((float)this->interpol_param * PI / INTERPOLATION_TIME_MS - HALF_PI) + 1.0) /
           2.0 * (float)(this->target.g - this->start.g) +
       this->start.g);
  this->current.b = (u_int8_t)
      ((sin((float)this->interpol_param * PI / INTERPOLATION_TIME_MS - HALF_PI) + 1.0) /
           2.0 * (float)(this->target.b - this->start.b) +
       this->start.b);
  //Serial.println("====================");
  //Serial.println(this->start.r);
  //Serial.println(this->current.r);
  //Serial.println(this->target.r);
  //Serial.println(this->interpol_param);
}

void Led::setTargetColor(RGB color) {
  if (this->target.r == color.r && this->target.g == color.g &&
      this->target.b == color.b) {
    return;
  }
  this->target = color;
  this->start = this->current;
  //Serial.println("Reset interpol param");
  this->interpol_param = 0;
  this->interpol_active = true;
}

bool Led::raiseInterplateParam(u_int16_t value) {
  if (this->interpol_param + value > INTERPOLATION_TIME_MS) {
    this->interpol_active = false;
    return false;
  }
  this->interpol_param += value;
  return true;
}

RGB Led::getColor() {
  return this->current;
}

RGB Led::getTargetColor() {
  return this->target;
}
#endif
