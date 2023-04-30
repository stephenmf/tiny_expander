#ifndef APP_VALVE_H
#define APP_VALVE_H

#include "io/gpio.h"
#include "pico/time.h"

template <uint pin, bool active_high>
class Valve {
 public:
  Valve() : next_{0} {}

  auto set(bool level) { pin_.set((level) ? active_high : !active_high); }
  auto get() {
    auto value = pin_.get();
    return (active_high) ? value : !value;
  }
  constexpr auto get_pin() { return pin_.get_pin(); }

  auto init() {
    pin_.init();
    set(false);
  }

  auto periodic() {
    if (next_ != 0 && next_ < time_us_64()) {
      set(false);
      next_ = 0;
    }
  }

  auto pulse(unsigned on_duration_sec) {
    if (on_duration_sec == 0) {
      set(false);
      next_ = 0;
    } else {
      set(true);
      next_ =
          (static_cast<uint64_t>(on_duration_sec) * 1000000UL) + time_us_64();
    }
  }

 private:
  uint64_t next_;
  Gpio<pin, true> pin_;
};

#endif  // APP_VALVE_H
