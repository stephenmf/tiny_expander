#include "io/console.h"

#include <cstdarg>

Console::Console(const char *banner, const Command *commands)
    : banner_{banner},
      commands_{commands},
      parse_state_{ParseState::CHARACTER},
      tx_index_{0},
      tx_sent_{0} {
  const char *scan = banner;
  while (*scan != '\0') output_buffer_[tx_index_++] = *scan++;
  edit_.reset();
}

auto Console::read_buffer() -> std::pair<uint8_t *, size_t> {
  return std::pair<uint8_t *, size_t>{rx_buffer_, sizeof(rx_buffer_)};
}

auto Console::read_done(size_t length) -> void {
  for (auto i = 0U; i < length; ++i) {
    auto c = rx_buffer_[i];
    auto action = parse(c);
    if (edit(action, c)) {
      printf("\r\n");
      edit_.buffer[edit_.end] = '\0';
      tokenise();
      if (command_line_.number > 0) {
        auto command = find_command(command_line_.parameters[0]);
        if (command && command->handler) command->handler(*this, command_line_);
      }
      edit_.reset();
      parse_state_ = ParseState::CHARACTER;
      printf("GH> ");
    }
  }
}

auto Console::write_buffer() -> std::pair<const char *, size_t> {
  size_t size = 0;
  size_t send_index = tx_sent_;
  // If buffer has been wrapped this is true.
  if (tx_index_ < tx_sent_)
    size = sizeof(output_buffer_) - tx_sent_;
  else
    size = tx_index_ - tx_sent_;
  return std::pair<const char *, size_t>{&output_buffer_[send_index], size};
}

auto Console::write_done(size_t length) -> void {
  size_t new_sent = tx_sent_ + length;
  if (new_sent >= sizeof(output_buffer_)) new_sent = 0;
  tx_sent_ = new_sent;
}

auto Console::putc(char c) -> bool {
  auto new_index = tx_index_ + 1;
  // wrap the buffer.
  if (new_index >= sizeof(output_buffer_)) new_index = 0;
  // Check for buffer full.
  if (new_index == tx_sent_) return false;
  output_buffer_[tx_index_] = c;
  tx_index_ = new_index;
  return true;
}

auto Console::printf(FORMAT format...) -> int {
  va_list args;
  va_start(args, format);
  int result = vprintf(format, args);
  va_end(args);
  return result;
}

auto Console::vprintf(FORMAT format, va_list args) -> int {
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

auto Console::edit(KeyAction action, char c) -> bool {
  switch (action) {
    case KeyAction::NONE:
      break;
    case KeyAction::PROCESS:
      return true;
      break;
    case KeyAction::ADD:
      if (edit_.end < (sizeof(edit_.buffer) - 1) &&
          edit_.cursor < (sizeof(edit_.buffer) - 1)) {
        for (auto i = edit_.end; i > edit_.cursor; --i)
          edit_.buffer[i] = edit_.buffer[i - 1];
        auto index = edit_.cursor;
        edit_.buffer[edit_.cursor++] = c;
        edit_.buffer[++edit_.end] = '\0';
        while (index < edit_.end) putc(edit_.buffer[index++]);
        while (index > edit_.cursor) {
          putc('\b');
          --index;
        }
      }
      break;
    case KeyAction::BACKSPACE:
      if (edit_.end > 0 && edit_.cursor > 0) {
        for (auto i = edit_.cursor - 1; i < edit_.end; ++i)
          edit_.buffer[i] = edit_.buffer[i + 1];
        --edit_.cursor;
        --edit_.end;
        edit_.buffer[edit_.end] = '\0';
        putc('\b');
        auto index = edit_.cursor;
        while (index < edit_.end) putc(edit_.buffer[index++]);
        putc(' ');
        while (index > edit_.cursor) {
          putc('\b');
          --index;
        }
        putc('\b');
      }
      break;
    case KeyAction::DELETE:
      if (edit_.cursor < edit_.end) {
        for (auto i = edit_.cursor; i < edit_.end; ++i)
          edit_.buffer[i] = edit_.buffer[i + 1];
        --edit_.end;
        edit_.buffer[edit_.end] = '\0';
        auto index = edit_.cursor;
        while (index < edit_.end) putc(edit_.buffer[index++]);
        putc(' ');
        while (index > edit_.cursor) {
          putc('\b');
          --index;
        }
        putc('\b');
      }
      break;
    case KeyAction::CURSOR_LEFT:
      if (edit_.cursor > 0) {
        --edit_.cursor;
        printf("\e[D");
      }
      break;
    case KeyAction::CURSOR_RIGHT:
      if (edit_.cursor < edit_.end) {
        ++edit_.cursor;
        printf("\e[C");
      }
      break;
    case KeyAction::CURSOR_HOME:
      while (edit_.cursor > 0) {
        --edit_.cursor;
        printf("\e[D");
      }
      break;
    case KeyAction::CURSOR_END:
      while (edit_.cursor < edit_.end) {
        ++edit_.cursor;
        printf("\e[C");
      }
      break;
  }
  return false;
}

auto Console::tokenise() -> void {
  command_line_.number = 0;
  size_t index = 0;
  auto number = 0;
  while (index < edit_.end) {
    // Strip spaces from the front
    while (index < edit_.end && edit_.buffer[index] == ' ') ++index;
    auto start = index;
    // '{' and '[' indicate the start of json parameter capture a single entity.
    if (edit_.buffer[start] == '{' || edit_.buffer[start] == '[') {
      auto brackets = 1;
      while (index < edit_.end && brackets > 0) {
        auto c = edit_.buffer[index++];
        if (c == '{' || c == '[')
          ++brackets;
        else if (c == '}' || c == ']')
          --brackets;
      }
    } else {
      while (index < edit_.end && edit_.buffer[index] != ' ') ++index;
    }
    command_line_.parameters[number].size = index - start;
    command_line_.parameters[number].token = &edit_.buffer[start];
    ++number;
    // If we have filled the parameters the last one is the rest of the line.
    if (number == CommandLine::MAX_PARAMETERS) {
      command_line_.parameters[number - 1].size = edit_.end - start;
      index = edit_.end;
    }
  }
  command_line_.number = number;
}

auto Console::find_command(Parameter &first) -> const Command * {
  auto scan = commands_;
  while (scan && scan->name && scan->handler) {
    bool found = true;
    size_t index = 0;
    while (found && index < first.size) {
      if (first.token[index] != scan->name[index]) found = false;
      ++index;
    }
    if (found) return scan;
    ++scan;
  }
  return nullptr;
}

auto Console::parse(char c) -> KeyAction {
  if (c == '\r') return KeyAction::PROCESS;
  if (parse_state_ == ParseState::CHARACTER) return parse_character(c);
  return parse_escape_sequence(c);
}

auto Console::parse_character(char c) -> KeyAction {
  auto result = KeyAction::NONE;
  // Translate tab into a space.
  if (c == '\t') c = ' ';
  if (c == '\e') {
    parse_state_ = ParseState::ESCAPED;
  } else if (c == '\b') {
    result = KeyAction::BACKSPACE;
  } else if (c >= ' ' && c < 127) {
    result = KeyAction::ADD;
  } else
    printf("|0x%02X|", c);
  return result;
}

auto Console::parse_escape_sequence(char c) -> KeyAction {
  auto result = KeyAction::NONE;
  if (c >= ' ' && c < 127) {
    switch (parse_state_) {
      case ParseState::ESCAPED:
        if (c == '[')
          parse_state_ = ParseState::ESCAPED_SQBKT;
        else if (c == 'O')
          parse_state_ = ParseState::ESCAPED_END;
        else {
          printf(" <Esc%c ", c);
          parse_state_ = ParseState::CHARACTER;
        }
        break;
      case ParseState::ESCAPED_SQBKT:
        if (c == 'A') {
          printf(" <UP> ");
          parse_state_ = ParseState::CHARACTER;
        } else if (c == 'B') {
          printf(" <DOWN> ");
          parse_state_ = ParseState::CHARACTER;
        } else if (c == 'C') {
          result = KeyAction::CURSOR_RIGHT;
          parse_state_ = ParseState::CHARACTER;
        } else if (c == '1') {
          parse_state_ = ParseState::ESCAPED_HOME;
        } else if (c == '3') {
          parse_state_ = ParseState::ESCAPED_DELETE;
        } else if (c == 'D') {
          parse_state_ = ParseState::CHARACTER;
          result = KeyAction::CURSOR_LEFT;
        } else {
          printf(" <Esc[%c ", c);
          parse_state_ = ParseState::CHARACTER;
        }
        break;
      case ParseState::ESCAPED_END:
        if (c == 'F')
          result = KeyAction::CURSOR_END;
        else {
          printf(" <EscO%c ", c);
        }
        parse_state_ = ParseState::CHARACTER;
        break;
      case ParseState::ESCAPED_DELETE:
        if (c == '~')
          result = KeyAction::DELETE;
        else {
          printf(" <Esc[3%c ", c);
        }
        parse_state_ = ParseState::CHARACTER;
        break;
      case ParseState::ESCAPED_HOME:
        if (c == '~')
          result = KeyAction::CURSOR_HOME;
        else {
          printf(" <Esc[1%c ", c);
        }
        parse_state_ = ParseState::CHARACTER;
        break;
      default:
        parse_state_ = ParseState::CHARACTER;
        printf(" <Esc?%c> ", c);
        break;
    }
  } else {
    printf(" <Esc?0x%02X> ", c);
    parse_state_ = ParseState::CHARACTER;
  }
  return result;
}

char Console::output_buffer_[OUTPUT_BUFFER_SIZE];
Console::EditBuffer Console::edit_;
uint8_t Console::rx_buffer_[RX_BUFFER_SIZE];
Conversion Console::conversion_;
Console::CommandLine Console::command_line_;
