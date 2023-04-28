#include "io/led.h"

#include "pico/time.h"

auto Led::init(uint pin, bool on) -> void {
  // Default LED gpio initialisation.
  pin_ = pin;
  on_ = on;
  on_duration_ms_ = 0;
  off_duration_ms_ = 0;
  next_ = 0;
  gpio_init(pin_);
  gpio_set_dir(pin_, GPIO_OUT);
  set(false);
}

auto Led::config(unsigned on_duration_ms, unsigned off_duration_ms) -> void {
  if (on_duration_ms == 0) {
    set(off_duration_ms != 0);
    next_ = 0;
    off_duration_ms_ = 0;
  } else {
    set(true);
    next_ = (static_cast<uint64_t>(on_duration_ms) * 1000UL) + time_us_64();
    off_duration_ms_ = off_duration_ms;
  }
  on_duration_ms_ = on_duration_ms;
}

auto Led::periodic() -> void {
  if (next_ != 0 && next_ < time_us_64()) {
    if (get()) {
      set(false);
      if (off_duration_ms_ != 0) {
        next_ =
            (static_cast<uint64_t>(off_duration_ms_) * 1000UL) + time_us_64();
      } else {
        next_ = 0;
      }
    } else {
      if (on_duration_ms_ != 0) {
        set(true);
        next_ =
            (static_cast<uint64_t>(on_duration_ms_) * 1000UL) + time_us_64();
      } else {
        next_ = 0;
      }
    }
  }
}
