#ifndef IO_LED_H
#define IO_LED_H

#include "io/gpio.h"
#include "pico/time.h"

template <uint pin, bool active_high>
class Led {
 public:
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

 private:
  Gpio<pin, true> pin_;
};

#endif  // IO_LED_H
