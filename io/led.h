#ifndef IO_LED_H
#define IO_LED_H

#include "io/gpio.h"
#include "pico/time.h"

template <uint pin, bool active_high>
class Led {
 public:
  Led() : on_duration_ms_{0}, off_duration_ms_{0}, next_{0} {}

  auto set(bool level) { pin_.set((level) ? active_high : !active_high); }
  auto get() {
    auto value = pin_.get();
    return (active_high) ? value : !value;
  }
  constexpr auto get_pin() { return pin_.get_pin(); }

  auto init(bool on) {
    pin_.init();
    set(on);
  }

  auto periodic() {
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

  auto config(unsigned on_duration_ms, unsigned off_duration_ms) {
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

 private:
  uint64_t next_;
  unsigned on_duration_ms_;
  unsigned off_duration_ms_;
  Gpio<pin, true> pin_;
};

#endif  // IO_LED_H
