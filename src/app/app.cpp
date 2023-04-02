#include "app/app.h"

#include "app_config.h"
#include "hardware/watchdog.h"
#include "io/framework.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"

namespace {

using FORMAT = const char *;

constexpr unsigned RESET_DELAY_MS = 100;

constexpr size_t output_bufferSIZE = 2048;
static char output_buffer[output_bufferSIZE];

constexpr size_t RX_BUFFER_SIZE = 64 + 1;
static uint8_t rx_buffer[RX_BUFFER_SIZE];

static Conversion conversion;
static size_t tx_index;
static size_t tx_sent;

auto respond_putc(char c) -> bool {
  auto new_index = tx_index + 1;
  // wrap the buffer.
  if (new_index >= sizeof(output_buffer)) {
    new_index = 0;
  }
  // Check for buffer full.
  if (new_index == tx_sent) {
    return false;
  }
  output_buffer[tx_index] = c;
  tx_index = new_index;
  return true;
}

auto vrespond(FORMAT format, va_list args) -> int {
  int result;
  bool room = true;
  while (room && *format != '\0') {
    if (*format == '%') {
      ++format;
      auto type = conversion.parse(format);
      if (type == Conversion::Type::UNKNOWN) {
        break;
      }
      const char *buffer = nullptr;
      switch (type) {
        case Conversion::Type::PERCENT: {
          buffer = conversion.from_character('%');
        } break;
        case Conversion::Type::CHARACTER: {
          auto value = va_arg(args, int);
          buffer = conversion.from_character(value);
        } break;
        case Conversion::Type::SIGNED_INT: {
          auto value = va_arg(args, int);
          buffer = conversion.from_signed_int(value);
        } break;
        case Conversion::Type::UNSIGNED_INT: {
          auto value = va_arg(args, unsigned);
          buffer = conversion.from_unsigned_int(value);
        } break;
        case Conversion::Type::LONG_SIGNED_INT: {
          auto value = va_arg(args, long);
          buffer = conversion.from_signed_int(value);
        } break;
        case Conversion::Type::LONG_UNSIGNED_INT: {
          auto value = va_arg(args, unsigned long);
          buffer = conversion.from_unsigned_int(value);
        } break;
        case Conversion::Type::LONG_LONG_SIGNED_INT: {
          auto value = va_arg(args, long long);
          buffer = conversion.from_signed_int(value);
        } break;
        case Conversion::Type::LONG_LONG_UNSIGNED_INT: {
          auto value = va_arg(args, unsigned long long);
          buffer = conversion.from_unsigned_int(value);
        } break;
        case Conversion::Type::POINTER: {
          auto value = (uint32_t)va_arg(args, void *);
          buffer = conversion.from_unsigned_int(value);
        } break;
        case Conversion::Type::DOUBLE: {
          auto value = va_arg(args, double);
          buffer = conversion.from_double(value);
        } break;
        case Conversion::Type::STRING: {
          auto value = va_arg(args, char *);
          buffer = conversion.from_string(value);
        } break;
        default:
          break;
      }
      if (buffer) {
        while (room && *buffer != '\0') {
          room = respond_putc(*buffer++);
        }
      }
    } else {
      room = respond_putc(*format++);
      ++result;
    }
  }
  return result;
}

auto respond(FORMAT format...) -> int {
  va_list args;
  va_start(args, format);
  int result = vrespond(format, args);
  va_end(args);
  return result;
}

struct Parser {
  enum class State { COMMAND, TARGET, NEXT_VALUE, VALUE };
  enum class Command { NONE, STATUS, RESET, VALVE, LED };

  Parser()
      : state{State::COMMAND}, command{Command::NONE}, target{0}, value{0} {}
  auto reset() -> void;
  auto parse(char c) -> bool;

  State state;
  Command command;
  uint8_t target;
  uint16_t value;
};

auto Parser::reset() -> void {
  state = Parser::State::COMMAND;
  command = Parser::Command::NONE;
  target = 0;
  value = 0;
}

auto Parser::parse(char c) -> bool {
  switch (state) {
    case State::COMMAND:
      if (c == 's' || c == 'S') {
        command = Command::STATUS;
        return true;
      } else if (c == 'l' || c == 'L') {
        command = Command::LED;
        state = State::NEXT_VALUE;
      } else if (c == 'r' || c == 'R') {
        command = Command::RESET;
        state = State::NEXT_VALUE;
      } else if (c == 'v' || c == 'v') {
        command = Command::VALVE;
        state = State::TARGET;
      } else if (c > ' ') {
        respond("Ec'%c'\r\n", c);
        reset();
      }
      break;
    case State::TARGET:
      if (c == 27) {
        reset();
      } else if (c >= '0' && c <= '9') {
        target = static_cast<uint8_t>(c - '0');
        state = State::NEXT_VALUE;
      } else if (c > ' ') {
        respond("Et'%c'\r\n", c);
        reset();
      }
      break;
    case State::NEXT_VALUE:
      if (c == 27) {
        reset();
      } else if (c >= '0' && c <= '9') {
        value = static_cast<uint16_t>(c - '0');
        state = State::VALUE;
      }
      break;
    case State::VALUE:
      if (c == 27) {
        reset();
      } else if (c >= '0' && c <= '9') {
        value = value * 10 + static_cast<uint16_t>(c - '0');
      } else {
        return true;
      }
      break;
  }
  return false;
}

static Parser parser{};

}  // namespace

App::App(Framework &framework)
    : framework_{framework}, led_{}, valve0_{}, valve1_{} {
  tx_index = 0;
  tx_sent = 0;
}

auto App::init() -> void {
  bi_decl(bi_1pin_with_name(LED_PIN, "LED"));
  led_.init(LED_PIN);

  bi_decl(bi_1pin_with_name(LED_PIN, "VALVE0"));
  valve0_.init(VALVE0_PIN);

  bi_decl(bi_1pin_with_name(LED_PIN, "VALVE1"));
  valve1_.init(VALVE1_PIN);
}

auto App::periodic() -> void {
  led_.periodic();
  valve0_.periodic();
  valve1_.periodic();
}

auto App::perform_command() -> void {
  auto &console = framework_.console();
  switch (parser.command) {
    case Parser::Command::STATUS:
      console.printf("Status\r\n");
      respond("R{\"l\":%d,\"v0\":%d,\"v1\":%d}\r\n", led_.get(), valve0_.get(),
              valve1_.get());
      break;
    case Parser::Command::RESET:
      console.printf("Reset value: %d\r\n", parser.value);
      if (parser.value == 5511) {
        // Reset to allow loading new image as if BOOTSEL was being held down.
        reset_usb_boot(0, 0);
      } else if (parser.value == 1033) {
        watchdog_reboot(0, 0, RESET_DELAY_MS);
      } else {
        respond("Er%d\r\n", parser.value);
      }
      break;
    case Parser::Command::LED:
      console.printf("Led update value: %d\r\n", parser.value);
      led_.config(parser.value, parser.value);
      respond("AL\r\n");
      break;
    case Parser::Command::VALVE:
      console.printf("Valve target: %d value: %d\r\n", parser.target,
                     parser.value);
      if (parser.target == 0) {
        valve0_.pulse(parser.value);
        respond("AV0\r\n");
      } else if (parser.target == 1) {
        valve1_.pulse(parser.value);
        respond("AV1\r\n");
      } else {
        respond("Ev%d\r\n", parser.target);
      }
      break;
  }
}

auto App::parse(char c) -> void {
  if (parser.parse(c)) {
    perform_command();
    parser.reset();
  }
}

auto App::read_buffer() -> std::pair<uint8_t *, size_t> {
  return std::pair<uint8_t *, size_t>{rx_buffer, sizeof(rx_buffer)};
}

auto App::read_done(size_t length) -> void {
  for (auto i = 0U; i < length; ++i) {
    auto c = rx_buffer[i];
    parse(c);
  }
}

auto App::write_buffer() -> std::pair<const char *, size_t> {
  size_t size = 0;
  size_t send_index = tx_sent;
  // If buffer has been wrapped this is true.
  if (tx_index < tx_sent) {
    size = sizeof(output_buffer) - tx_sent;
  } else {
    size = tx_index - tx_sent;
  }
  return std::pair<const char *, size_t>{&output_buffer[send_index], size};
}

auto App::write_done(size_t length) -> void {
  size_t new_sent = tx_sent + length;
  if (new_sent >= sizeof(output_buffer)) {
    new_sent = 0;
  }
  tx_sent = new_sent;
}
