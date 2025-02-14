#ifndef APP_INDICATOR_H
#define APP_INDICATOR_H

#include "io/led.h"

enum class State {
  DISCONNECTED = 0,
  CONNECTED,
  VALVE0_ON,
  VALVE1_ON,
  BOTH_VALVES_ON
};

template <uint red_pin, uint grn_pin, uint blu_pin, bool active_high>
class Indicator {
 public:
  Indicator()
      : state_{State::DISCONNECTED},
        on_{false},
        next_{0},
        on_duration_ms_{750},
        off_duration_ms_{250} {}
  constexpr auto get_red_pin() { return led_red_.get_pin(); }
  constexpr auto get_grn_pin() { return led_grn_.get_pin(); }
  constexpr auto get_blu_pin() { return led_blu_.get_pin(); }

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
    }
  }

  auto get_state() { return static_cast<uint8_t>(state_); }

  auto init(bool on) {
    led_red_.init(on);
    led_grn_.init(on);
    led_blu_.init(on);
    set_state(State::DISCONNECTED);
  }

  auto periodic() {
    auto now = time_us_64();
    if (next_ < now) {
      if (on_) {
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
      on_ = !on_;
    }
  }

 private:
  auto set(bool on) {
    if (on) {
      switch (state_) {
        case State::DISCONNECTED:
          led_red_.set(true);
          led_grn_.set(false);
          led_blu_.set(false);
          break;
        case State::CONNECTED:
          led_red_.set(false);
          led_grn_.set(false);
          led_blu_.set(false);
          break;
        case State::VALVE1_ON:
          led_red_.set(false);
          led_grn_.set(false);
          led_blu_.set(true);
          break;
        case State::VALVE0_ON:
        case State::BOTH_VALVES_ON:
          led_red_.set(false);
          led_grn_.set(true);
          led_blu_.set(false);
          break;
      }
    } else {
      switch (state_) {
        case State::DISCONNECTED:
        case State::CONNECTED:
        case State::VALVE0_ON:
        case State::VALVE1_ON:
          led_red_.set(false);
          led_grn_.set(false);
          led_blu_.set(false);
          break;
        case State::BOTH_VALVES_ON:
          led_red_.set(false);
          led_grn_.set(false);
          led_blu_.set(true);
          break;
      }
    }
  }

  State state_;
  bool on_;
  uint64_t next_;
  unsigned on_duration_ms_;
  unsigned off_duration_ms_;
  Led<red_pin, active_high> led_red_;
  Led<grn_pin, active_high> led_grn_;
  Led<blu_pin, active_high> led_blu_;
};

#endif  // APP_INDICATOR_H
