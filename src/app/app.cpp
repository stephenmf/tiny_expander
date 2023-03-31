#include "app/app.h"

#include "app_config.h"
#include "hardware/watchdog.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"

namespace {

constexpr unsigned RESET_DELAY_MS = 100;

// int64_t alarm_callback(alarm_id_t id, void *user_data) {
//   // Put your timeout handler code in here
//   return 0;
// }

}  // namespace

App::App()
    : state_{State::WAIT_COMMAND},
      led_{},
      valve0_{},
      valve1_{},
      command_{Command::STATUS},
      index_{0},
      collect_{0},
      tx_index_{0},
      tx_sent_{0} {}

auto App::init() -> void {
  bi_decl(bi_1pin_with_name(LED_PIN, "LED"));
  led_.init(LED_PIN);

  bi_decl(bi_1pin_with_name(LED_PIN, "VALVE0"));
  valve0_.init(VALVE0_PIN);

  bi_decl(bi_1pin_with_name(LED_PIN, "VALVE1"));
  valve1_.init(VALVE1_PIN);

#if 0
  // SPI initialisation. This example will use SPI at 1MHz.
  bi_decl(bi_3pins_with_func(PIN_MISO, PIN_SCK, PIN_MOSI, GPIO_FUNC_SPI));
  bi_decl(bi_1pin_with_name(PIN_CS, "SPI0 CS"));
  spi_init(SPI_PORT, 1000 * 1000);

  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_set_dir(PIN_CS, GPIO_OUT);
  gpio_put(PIN_CS, 1);
#endif

#if 0
  // I2C Initialisation. Using it at 400Khz.
  bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
#endif
}

auto App::periodic() -> void {
  led_.periodic();
  valve0_.periodic();
  valve1_.periodic();
}

/*
pub enum Commands {
    Status,
    Valve,
    Led,
}

impl fmt::Display for Commands {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Commands::Status => write!(f, "Status"),
            Commands::Led => write!(f, "Led"),
            Commands::Valve => write!(f, "Valve"),
        }
    }
}

enum DecodeState {
    Command,
    Target,
    NextValue,
    Value,
}

pub enum DecodeResult {
    None,
    Text(String<64>),
    Command(Commands, u8, u16),
}

pub struct Decoder {
    state: DecodeState,
    target: u8,
    value: u16,
    command: Commands,
}

impl Decoder {
    pub fn new() -> Decoder {
        Decoder {
            state: DecodeState::Command,
            target: 0,
            value: 0,
            command: Commands::Status,
        }
    }
    pub fn run(&mut self, c: &u8) -> DecodeResult {
        match self.state {
            DecodeState::Command => match c {
                b's' | b'S' => return DecodeResult::Command(Commands::Status, 0, 0),
                b'v' | b'V' => {
                    self.command = Commands::Valve;
                    self.state = DecodeState::Target
                }
                b'l' | b'L' => {
                    self.command = Commands::Led;
                    self.state = DecodeState::NextValue
                }
                // ignore control codes.
                0..=31 => {}
                _ => {
                    let mut text: String<64> = String::new();
                    writeln!(text, "Err: unrecognised '{c}'\r").unwrap();
                    return DecodeResult::Text(text);
                }
            },
            DecodeState::Target => match c {
                // Esc cancel command
                27 => self.state = DecodeState::Command,
                b'0'..=b'9' => {
                    self.target = c - b'0';
                    self.state = DecodeState::NextValue
                }
                // ignore control codes.
                0..=31 => {}
                _ => {
                    let mut text: String<64> = String::new();
                    writeln!(text, "Err: bad target '{c}'\r").unwrap();
                    self.state = DecodeState::Command;
                    return DecodeResult::Text(text);
                }
            },
            DecodeState::NextValue => match c {
                // Esc cancel command
                27 => self.state = DecodeState::Command,
                b'0'..=b'9' => {
                    self.value = (c - b'0') as u16;
                    self.state = DecodeState::Value
                }
                _ => {}
            },
            DecodeState::Value => match c {
                // Esc cancel command
                27 => self.state = DecodeState::Command,
                b'0'..=b'9' => {
                    self.value = self.value * 10 + (c - b'0') as u16;
                    self.state = DecodeState::Value
                }
                _ => {
                    self.state = DecodeState::Command;
                    return DecodeResult::Command(self.command, self.target, self.value);
                }
            },
        }
        DecodeResult::None
    }
}
*/
auto App::perform_command(Command command) -> void {
  switch (command) {
    case Command::STATUS:
      printf("J{\"led\":%d,\"valve0\":%d,\"valve0\":%d}\r\n", led_.get(),
             valve0_.get(), valve1_.get());
      break;
    case Command::RESET:
      if (params_[0] == 55, params_[1] == 11) {
        // Reset to allow loading new image as if BOOTSEL was being held down.
        reset_usb_boot(0, 0);
      } else if (params_[0] == 10, params_[1] == 33) {
        watchdog_reboot(0, 0, RESET_DELAY_MS);
      }
      break;
    case Command::UPDATE_LED:
      led_.config(params_[0], params_[1]);
      printf("A\r\n");
      // Timer example code - This example fires off the callback after 2000ms
      // add_alarm_in_ms(2000, alarm_callback, NULL, false);
      break;
    case Command::VALVE_PULSE:
      if (params_[0] == 0) {
        valve0_.pulse(params_[1]);
        printf("A\r\n");
      } else if (params_[0] == 1) {
        valve1_.pulse(params_[1]);
        printf("A\r\n");
      } else {
        printf("N\r\n");
      }
      // Timer example code - This example fires off the callback after 2000ms
      // add_alarm_in_ms(2000, alarm_callback, NULL, false);
      break;
  }
}

auto App::parse(char c) -> void {
  switch (state_) {
    case State::WAIT_COMMAND:
      wait_command(c);
      break;
    case State::WAIT_PARAMS:
      wait_params(c);
      break;
    case State::COLLECT_PARAMS:
      collect_params(c);
      break;
  }
  printf("state: %d response: 0x%02X\r\n", state_, (int)c);
}

auto App::wait_command(char c) -> void {
  switch (c) {
    case 0x1B:
    case '\r':
    case '\n':
      state_ = State::WAIT_COMMAND;
      break;
    case 'S':
    case 's':
      command_ = Command::STATUS;
      perform_command(command_);
      state_ = State::WAIT_COMMAND;
      break;
    case 'R':
      command_ = Command::RESET;
      index_ = 0;
      collect_ = 2;
      state_ = State::WAIT_PARAMS;
      break;
    case 'V':
    case 'v':
      command_ = Command::VALVE_PULSE;
      index_ = 0;
      collect_ = 2;
      state_ = State::WAIT_PARAMS;
      break;
    case 'L':
    case 'l':
      command_ = Command::UPDATE_LED;
      index_ = 0;
      collect_ = 2;
      state_ = State::WAIT_PARAMS;
      break;
    default:
      printf("U'%c'\r\n", c);
      break;
  }
}

auto App::wait_params(char c) -> void {
  switch (c) {
    case 0x1B:
      state_ = State::WAIT_COMMAND;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      params_[index_] = c - '0';
      state_ = State::COLLECT_PARAMS;
      break;
    default:
      break;
  }
}

auto App::collect_params(char c) -> void {
  switch (c) {
    case 0x1B:
      state_ = State::WAIT_COMMAND;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      params_[index_] = (params_[index_] * 10) + (c - '0');
      break;
    default:
      ++index_;
      if (index_ >= collect_) {
        perform_command(command_);
        state_ = State::WAIT_COMMAND;
      } else {
        state_ = State::WAIT_PARAMS;
      }
      break;
  }
}

auto App::read_buffer() -> std::pair<uint8_t *, size_t> {
  return std::pair<uint8_t *, size_t>{rx_buffer_, sizeof(rx_buffer_)};
}

auto App::read_done(size_t length) -> void {
  for (auto i = 0U; i < length; ++i) {
    auto c = rx_buffer_[i];
    parse(c);
  }
}

auto App::write_buffer() -> std::pair<const char *, size_t> {
  size_t size = 0;
  size_t send_index = tx_sent_;
  // If buffer has been wrapped this is true.
  if (tx_index_ < tx_sent_)
    size = sizeof(output_buffer_) - tx_sent_;
  else
    size = tx_index_ - tx_sent_;
  return std::pair<const char *, size_t>{&output_buffer_[send_index], size};
}

auto App::write_done(size_t length) -> void {
  size_t new_sent = tx_sent_ + length;
  if (new_sent >= sizeof(output_buffer_)) new_sent = 0;
  tx_sent_ = new_sent;
}

auto App::putc(char c) -> bool {
  auto new_index = tx_index_ + 1;
  // wrap the buffer.
  if (new_index >= sizeof(output_buffer_)) new_index = 0;
  // Check for buffer full.
  if (new_index == tx_sent_) return false;
  output_buffer_[tx_index_] = c;
  tx_index_ = new_index;
  return true;
}

auto App::printf(FORMAT format...) -> int {
  va_list args;
  va_start(args, format);
  int result = vprintf(format, args);
  va_end(args);
  return result;
}

auto App::vprintf(FORMAT format, va_list args) -> int {
  int result;
  bool room = true;
  while (room && *format != '\0') {
    if (*format == '%') {
      ++format;
      auto type = conversion_.parse(format);
      if (type == Conversion::Type::UNKNOWN) break;
      const char *buffer = nullptr;
      switch (type) {
        case Conversion::Type::PERCENT: {
          buffer = conversion_.from_character('%');
        } break;
        case Conversion::Type::CHARACTER: {
          auto value = va_arg(args, int);
          buffer = conversion_.from_character(value);
        } break;
        case Conversion::Type::SIGNED_INT: {
          auto value = va_arg(args, int);
          buffer = conversion_.from_signed_int(value);
        } break;
        case Conversion::Type::UNSIGNED_INT: {
          auto value = va_arg(args, unsigned);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::LONG_SIGNED_INT: {
          auto value = va_arg(args, long);
          buffer = conversion_.from_signed_int(value);
        } break;
        case Conversion::Type::LONG_UNSIGNED_INT: {
          auto value = va_arg(args, unsigned long);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::LONG_LONG_SIGNED_INT: {
          auto value = va_arg(args, long long);
          buffer = conversion_.from_signed_int(value);
        } break;
        case Conversion::Type::LONG_LONG_UNSIGNED_INT: {
          auto value = va_arg(args, unsigned long long);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::POINTER: {
          auto value = (uint32_t)va_arg(args, void *);
          buffer = conversion_.from_unsigned_int(value);
        } break;
        case Conversion::Type::DOUBLE: {
          auto value = va_arg(args, double);
          buffer = conversion_.from_double(value);
        } break;
        case Conversion::Type::STRING: {
          auto value = va_arg(args, char *);
          buffer = conversion_.from_string(value);
        } break;
        default:
          break;
      }
      if (buffer) {
        while (room && *buffer != '\0') room = putc(*buffer++);
      }
    } else {
      room = putc(*format++);
      ++result;
    }
  }
  return result;
}

char App::output_buffer_[OUTPUT_BUFFER_SIZE];
uint8_t App::rx_buffer_[RX_BUFFER_SIZE];
Conversion App::conversion_;
