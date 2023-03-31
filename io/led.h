#ifndef IO_LED_H
#define IO_LED_H

#include "hardware/gpio.h"

class Led {
 public:
  auto init(uint pin) -> void;
  auto periodic() -> void;

  auto config(unsigned on_duration_ms, unsigned off_duration_ms) -> void;

  auto set(bool level) { gpio_put(pin_, level); }

  auto get() { return gpio_get(pin_); }

 private:
  uint pin_;
  unsigned on_duration_ms_;
  unsigned off_duration_ms_;
  uint64_t next_;
};

#endif  // IO_LED_H
