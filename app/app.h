#ifndef APP_APP_H
#define APP_APP_H

#include <cstdarg>

#include "app/valve.h"
#include "io/app_api.h"
#include "io/conversion.h"
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
  Led led_;
  Valve valve0_;
  Valve valve1_;
};

#endif  // APP_APP_H
