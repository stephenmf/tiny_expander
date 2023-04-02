#ifndef IO_FRAMEWORK_H
#define IO_FRAMEWORK_H

#include "io/console.h"

class AppApi;
class Framework {
 public:
  static Framework& get(const char* banner = nullptr,
                        const Console::Command* commands = nullptr);

  Framework(const char* banner, const Console::Command* commands)
      : app_{nullptr}, console_{banner, commands} {
    puts("Framework constructed");
  }

  auto init() -> void;
  auto periodic() -> void;

  // Accessors
  auto app(AppApi* app) { app_ = app; }
  auto app() { return app_; }
  auto& console() { return console_; }

 private:
  static constexpr uint8_t API_USB_CH = 0;
  static constexpr uint8_t DEBUG_USB_CH = 1;

  AppApi* app_;
  Console console_;
};

#endif  // IO_FRAMEWORK_H
