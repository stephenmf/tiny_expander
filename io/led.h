#ifndef IO_LED_H
#define IO_LED_H

#include "hardware/gpio.h"

class Led {
 public:
  Led() : pin_{0}, on_{0}, on_duration_ms_{0}, off_duration_ms_{0}, next_{0} {}

  auto init(uint pin, bool on) -> void;
  auto periodic() -> void;

  auto config(unsigned on_duration_ms, unsigned off_duration_ms) -> void;

  auto set(bool level) { gpio_put(pin_, (level) ? on_ : !on_); }

  auto get() { return (on_) ? gpio_get(pin_) : !gpio_get(pin_); }

 private:
  uint pin_;
  bool on_;
  unsigned on_duration_ms_;
  unsigned off_duration_ms_;
  uint64_t next_;
};

#endif  // IO_LED_H
