#ifndef APP_INDICATOR_H
#define APP_INDICATOR_H

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"

enum class State {
  DISCONNECTED = 0,
  CONNECTED,
  VALVE0_ON,
  VALVE1_ON,
  BOTH_VALVES_ON
};

template <uint red_pin, uint grn_pin, uint blu_pin>
class Indicator {
  static constexpr uint FADE = 60;
  static constexpr uint MIN_FADE = 6;
  static constexpr uint MAX_PHASE = 7;
  static constexpr uint16_t PWM_WRAP = 254;
  static constexpr uint RED_LEVEL = 64;
  static constexpr uint GRN_LEVEL = 64;
  static constexpr uint BLU_LEVEL = 192;

 public:
  Indicator()
      : state_{State::DISCONNECTED},
        phase_{0},
        next_{0},
        on_duration_ms_{750},
        off_duration_ms_{250},
        fade_{0} {}
  constexpr auto get_red_pin() { return red_pin; }
  constexpr auto get_grn_pin() { return grn_pin; }
  constexpr auto get_blu_pin() { return blu_pin; }

  auto set_state(State state) {
    if (state != state_) {
      state_ = state;
      switch (state_) {
        case State::DISCONNECTED:
          on_duration_ms_ = 750;
          off_duration_ms_ = 250;
          break;
        case State::CONNECTED:
          on_duration_ms_ = 250;
          off_duration_ms_ = 750;
          break;
        case State::VALVE0_ON:
        case State::VALVE1_ON:
        case State::BOTH_VALVES_ON:
          on_duration_ms_ = 500;
          off_duration_ms_ = 500;
          break;
      }
      fade_ = FADE;
    }
  }

  auto get_state() { return static_cast<uint8_t>(state_); }

  auto init(bool on) {
    set_state(State::DISCONNECTED);
    fade_ = FADE;

    // Initialise pwm.
    gpio_set_function(red_pin, GPIO_FUNC_PWM);
    gpio_set_function(grn_pin, GPIO_FUNC_PWM);
    gpio_set_function(blu_pin, GPIO_FUNC_PWM);

    auto red_slice = pwm_gpio_to_slice_num(red_pin);
    pwm_set_wrap(red_slice, PWM_WRAP);
    pwm_set_enabled(red_slice, true);

    auto grn_slice = pwm_gpio_to_slice_num(grn_pin);
    if (grn_slice != red_slice) {
      pwm_set_wrap(grn_slice, PWM_WRAP);
      pwm_set_enabled(grn_slice, true);
    }

    auto blu_slice = pwm_gpio_to_slice_num(blu_pin);
    if (blu_slice != red_slice && blu_slice != grn_slice) {
      pwm_set_wrap(blu_slice, PWM_WRAP);
      pwm_set_enabled(blu_slice, true);
    }

    pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
    pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
    pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
  }

  auto periodic() {
    auto now = time_us_64();
    if (next_ < now) {
      if ((phase_ & 1) == 0) {
        set(false);
        if (off_duration_ms_ != 0) {
          next_ = (static_cast<uint64_t>(off_duration_ms_) * 1000UL) + now;
        } else {
          next_ = 0;
        }
      } else {
        if (on_duration_ms_ != 0) {
          set(true);
          next_ = (static_cast<uint64_t>(on_duration_ms_) * 1000UL) + now;
        } else {
          next_ = 0;
        }
      }
      if (phase_ < MAX_PHASE)
        phase_ += 1;
      else
        phase_ = 0;
    }
  }

 private:
  auto set(bool on) {
    if (on) {
      switch (state_) {
        case State::DISCONNECTED:
          pwm_set_gpio_level(red_pin, on_level(RED_LEVEL));
          pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
          pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
          break;
        case State::CONNECTED:
          switch (phase_) {
            case 1:
              pwm_set_gpio_level(red_pin, on_level(RED_LEVEL));
              pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
              pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
              break;
            case 3:
              pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
              pwm_set_gpio_level(grn_pin, on_level(GRN_LEVEL));
              pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
              break;
            case 5:
              pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
              pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
              pwm_set_gpio_level(blu_pin, on_level(BLU_LEVEL));
              break;
            default:
              pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
              pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
              pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
              break;
          }
          if (fade_ > MIN_FADE) fade_ -= 1;
          break;
        case State::VALVE1_ON:
          pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
          pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
          pwm_set_gpio_level(blu_pin, on_level(BLU_LEVEL));
          break;
        case State::VALVE0_ON:
        case State::BOTH_VALVES_ON:
          pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
          pwm_set_gpio_level(grn_pin, on_level(GRN_LEVEL));
          pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
          break;
      }
    } else {
      switch (state_) {
        case State::DISCONNECTED:
        case State::CONNECTED:
        case State::VALVE0_ON:
        case State::VALVE1_ON:
          pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
          pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
          pwm_set_gpio_level(blu_pin, off_level(BLU_LEVEL));
          break;
        case State::BOTH_VALVES_ON:
          pwm_set_gpio_level(red_pin, off_level(RED_LEVEL));
          pwm_set_gpio_level(grn_pin, off_level(GRN_LEVEL));
          pwm_set_gpio_level(blu_pin, on_level(BLU_LEVEL));
          break;
      }
    }
  }

  auto on_level(uint on) -> uint16_t { return PWM_WRAP - (on * fade_) / FADE; }
  auto off_level(uint off) -> uint16_t { return PWM_WRAP + 1; }

  State state_;
  uint phase_;
  uint64_t next_;
  unsigned on_duration_ms_;
  unsigned off_duration_ms_;
  uint fade_;
};

#endif  // APP_INDICATOR_H
