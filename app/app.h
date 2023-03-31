#ifndef APP_APP_H
#define APP_APP_H

#include <cstdarg>

#include "app/valve.h"
#include "io/app_api.h"
#include "io/conversion.h"
#include "io/led.h"

class Framework;

class App : public AppApi {
  using FORMAT = const char*;

 public:
  enum class State { WAIT_COMMAND = 0, WAIT_PARAMS, COLLECT_PARAMS };
  enum class Command { STATUS = 0, RESET, VALVE_PULSE, UPDATE_LED };

  App();

  auto init() -> void override;
  auto periodic() -> void override;

  auto read_buffer() -> std::pair<uint8_t*, size_t> override;
  auto read_done(size_t length) -> void override;
  auto write_buffer() -> std::pair<const char*, size_t> override;
  auto write_done(size_t length) -> void override;

 private:
  static constexpr size_t OUTPUT_BUFFER_SIZE = 2048;
  static constexpr size_t RX_BUFFER_SIZE = 64 + 1;
  static constexpr size_t MAX_PARMS = 4;

  static Conversion conversion_;
  static char output_buffer_[OUTPUT_BUFFER_SIZE];
  static uint8_t rx_buffer_[RX_BUFFER_SIZE];

  auto putc(char c) -> bool;
  auto printf(FORMAT format...) -> int;
  auto vprintf(FORMAT format, va_list args) -> int;

  auto perform_command(Command command) -> void;
  auto parse(char c) -> void;
  auto wait_command(char c) -> void;
  auto wait_params(char c) -> void;
  auto collect_params(char c) -> void;
  auto execute(char c) -> void;

  State state_;
  Led led_;
  Valve valve0_;
  Valve valve1_;

  Command command_;
  unsigned params_[MAX_PARMS];
  unsigned index_;
  unsigned collect_;
  size_t tx_index_;
  size_t tx_sent_;
};

#endif  // APP_APP_H
