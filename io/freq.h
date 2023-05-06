#ifndef IO_FREQ_H
#define IO_FREQ_H

#include "hardware/pwm.h"
#include "pico/time.h"

template <uint pin>
class Freq {
 public:
  Freq() : next_{0}, value_{0} {}

  constexpr auto get_pin() { return pin; }

  auto init() {
    // Only the PWM B pins can be used as inputs.
    assert(pwm_gpio_to_channel(pin) == PWM_CHAN_B);
    next_ = time_us_64();
    auto cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    auto slice_num = pwm_gpio_to_slice_num(pin);
    pwm_init(slice_num, &cfg, false);
    gpio_set_function(pin, GPIO_FUNC_PWM);
  }

  auto periodic() {
    auto now = time_us_64();
    if (next_ < now) {
      static constexpr uint64_t NEXT_INCREMENT = 10 * 1000UL * 1000UL;
      uint slice_num = pwm_gpio_to_slice_num(pin);
      pwm_set_enabled(slice_num, false);
      value_ = pwm_get_counter(slice_num);
      updated_ = 1;
      pwm_set_counter(slice_num, 0);
      pwm_set_enabled(slice_num, true);
      next_ = NEXT_INCREMENT + now;
    }
  }

  auto value() { updated_ = 0; return value_; }
  auto updated() { return updated_; }

 private:
  uint64_t next_;
  uint16_t value_;
  uint8_t updated_;
};

#endif  // IO_LED_H
