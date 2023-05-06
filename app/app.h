#ifndef APP_APP_H
#define APP_APP_H

#include <cstdarg>

#include "app/valve.h"
#include "app_config.h"
#include "io/app_api.h"
#include "io/conversion.h"
#include "io/freq.h"
#include "io/led.h"

class Framework;

class App : public AppApi {
 public:
  App(Framework& framework);

  auto init() -> void override;
  auto periodic() -> void override;

  auto read_buffer() -> std::pair<uint8_t*, size_t> override;
  auto read_done(size_t length) -> void override;
  auto write_buffer() -> std::pair<const char*, size_t> override;
  auto write_done(size_t length) -> void override;

 private:
  auto perform_command() -> void;
  auto parse(char c) -> void;

  Framework& framework_;
  Led<LED_RED_PIN, LED_RED_ON> led_red_;
  Led<LED_GRN_PIN, LED_GRN_ON> led_grn_;
  Led<LED_BLU_PIN, LED_BLU_ON> led_blu_;
  Valve<VALVE0_PIN, VALVE0_ON> valve0_;
  Valve<VALVE1_PIN, VALVE1_ON> valve1_;
  Freq<MOISTURE_PIN, 10> moisture_;
  Freq<FLOW_PIN, 2> flow_;
};

#endif  // APP_APP_H
