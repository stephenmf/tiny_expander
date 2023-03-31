#include "io/framework.h"

#include "app_config.h"
#include "io/app_api.h"
#include "pico/binary_info.h"
#include "tusb.h"

Framework* Framework::get(const char* banner,
                          const Console::Command* commands) {
  static auto framework = Framework{banner, commands};
  return &framework;
}

#include "class/cdc/cdc_device.h"

#if 0
// Invoked when received new data
void tud_cdc_rx_cb(uint8_t itf) {
  printf("tud_cdc_rx_cb(%d)\r\n", itf);
  // auto* framework = Framework::get();
  // framework->console().printf("tud_cdc_rx_cb(%d)\r\n", itf);
}

// Invoked when received `wanted_char`
void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char) {
  printf("tud_cdc_rx_wanted_cb(%d)\r\n", itf);
  // auto* framework = Framework::get();
  // framework->console().printf("tud_cdc_rx_wanted_cb(%d)\r\n");
}

// Invoked when space becomes available in TX buffer
void tud_cdc_tx_complete_cb(uint8_t itf) {
  printf("tud_cdc_tx_complete_cb(%d)\r\n", itf);
  // if(itf == Framework::API_USB_CH) {
  // auto* framework = Framework::get();
  // framework->console().printf("tud_cdc_tx_complete_cb(%d)\r\n");
  // }
}

// Invoked when line state DTR & RTS are changed via SET_CONTROL_LINE_STATE
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  printf("tud_cdc_line_state_cb(%d, %d, %d)\r\n", itf, dtr, rts);
  // auto* framework = Framework::get();
  // framework->console().printf("tud_cdc_line_state_cb(%d)\r\n");
}

// Invoked when line coding is change via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf,
                            cdc_line_coding_t const* p_line_coding) {
  printf("tud_cdc_line_coding_cb(%d)\r\n", itf);
  // auto* framework = Framework::get();
  // framework->console().printf("tud_cdc_line_coding_cb(%d)\r\n");
}

// Invoked when received send break
void tud_cdc_send_break_cb(uint8_t itf, uint16_t duration_ms) {
  printf("tud_cdc_send_break_cb(%d)\r\n", itf);
  // auto* framework = Framework::get();
  // framework->console().printf("tud_cdc_send_break_cb(%d)\r\n");
}
#endif

auto Framework::init() -> void {
  // static uart_inst_t* uart_inst;
  if (app_) {
    app_->init();
  }

#if 0
  // stdio uart initialisation.
  bi_decl(bi_2pins_with_func(UART_TX_PIN, UART_TX_PIN, GPIO_FUNC_UART));
  uart_inst = uart_get_instance(UART_DEV);
  stdio_uart_init_full(uart_inst, CFG_BOARD_UART_BAUDRATE, UART_TX_PIN,
                       UART_RX_PIN);
#endif

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
