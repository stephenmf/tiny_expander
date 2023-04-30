#ifndef IO_GPIO_H
#define IO_GPIO_H

#include "hardware/gpio.h"

template <uint pin, bool out>
struct Gpio {
  auto set(bool level) { gpio_put(pin, level); }
  auto get() { return gpio_get(pin); }
  auto init() {
    gpio_init(pin);
    gpio_set_dir(pin, out);
  }
  constexpr auto get_pin() { return pin; }
};

#endif  // IO_GPIO_H
