#include "app/valve.h"

#include "pico/time.h"

auto Valve::init(uint pin) -> void {
  // Default LED gpio initialisation.
  pin_ = pin;
  next_ = 0;
  gpio_init(pin_);
  gpio_set_dir(pin_, GPIO_OUT);
}

auto Valve::pulse(unsigned on_duration_sec) -> void {
  if (on_duration_sec == 0) {
    set(false);
    next_ = 0;
  } else {
    set(true);
    next_ = (static_cast<uint64_t>(on_duration_sec) * 1000000UL) + time_us_64();
  }
}

auto Valve::periodic() -> void {
  if (next_ != 0 && next_ < time_us_64()) {
    set(false);
    next_ = 0;
  }
}
