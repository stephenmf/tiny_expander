#ifndef APP_VALVE_H
#define APP_VALVE_H

#include "hardware/gpio.h"

class Valve {
 public:
  auto init(uint pin) -> void;
  auto periodic() -> void;

  auto pulse(unsigned on_duration_sec) -> void;

  auto set(bool level) { gpio_put(pin_, level); }

  auto get() { return gpio_get(pin_); }

 private:
  uint pin_;
  uint64_t next_;
};

#endif  // APP_VALVE_H
