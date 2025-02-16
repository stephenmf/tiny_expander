#ifndef APP_APP_H
#define APP_APP_H

#include <cstdarg>

#include "app/indicator.h"
#include "app/valve.h"
#include "app_config.h"
#include "io/app_api.h"
#include "io/conversion.h"
#include "io/freq.h"

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
  Indicator<LED_RED_PIN, LED_GRN_PIN, LED_BLU_PIN> indicator_;
  Valve<VALVE0_PIN, VALVE0_ON> valve0_;
  Valve<VALVE1_PIN, VALVE1_ON> valve1_;
  Freq<MOISTURE0_PIN, 10> moisture0_;
  Freq<MOISTURE1_PIN, 10> moisture1_;
  Freq<FLOW0_PIN, 2> flow0_;
  Freq<FLOW1_PIN, 2> flow1_;
  uint64_t timeout_;
};

#endif  // APP_APP_H
