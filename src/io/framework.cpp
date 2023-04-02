#include "io/framework.h"

#include "io/app_api.h"
#include "tusb.h"

Framework& Framework::get(const char* banner,
                          const Console::Command* commands) {
  static auto framework = Framework{banner, commands};
  return framework;
}

#include "class/cdc/cdc_device.h"

auto Framework::init() -> void {
  if (app_) {
    app_->init();
  }
  tusb_init();
}

auto Framework::periodic() -> void {
  tud_task();
  // Check for input over cdc debug channel.
  if (tud_cdc_n_write_available(DEBUG_USB_CH)) {
    auto [out, size] = console_.write_buffer();
    auto sent = tud_cdc_n_write(DEBUG_USB_CH, out, size);
    console_.write_done(sent);
    tud_cdc_n_write_flush(DEBUG_USB_CH);
  }
  if (tud_cdc_n_available(DEBUG_USB_CH)) {
    auto [in, size] = console_.read_buffer();
    auto read = tud_cdc_n_read(DEBUG_USB_CH, in, size);
    console_.read_done(read);
  }
  // Check for input over cdc api channel.
  if (app_) {
    if (tud_cdc_n_write_available(API_USB_CH)) {
      auto [out, size] = app_->write_buffer();
      auto sent = tud_cdc_n_write(API_USB_CH, out, size);
      app_->write_done(sent);
      tud_cdc_n_write_flush(API_USB_CH);
    }
    if (tud_cdc_n_available(API_USB_CH)) {
      auto [in, size] = app_->read_buffer();
      auto read = tud_cdc_n_read(API_USB_CH, in, size);
      app_->read_done(read);
    }
    app_->periodic();
  }
}
