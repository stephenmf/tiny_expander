#ifndef IO_CONSOLE_H
#define IO_CONSOLE_H

#include <functional>
#include <cstdarg>

#include "io/conversion.h"

class Console {
  using FORMAT = const char *;

 public:
  enum class ParseState {
    CHARACTER = 0,
    ESCAPED,
    ESCAPED_SQBKT,
    ESCAPED_END,
    ESCAPED_HOME,
    ESCAPED_DELETE
  };

  enum class KeyAction {
    NONE = 0,
    ADD,
    BACKSPACE,
    DELETE,
    CURSOR_LEFT,
    CURSOR_RIGHT,
    CURSOR_HOME,
    CURSOR_END,
    PROCESS
  };

  struct Parameter {
    size_t size;
    const char *token;
  };

  struct CommandLine {
    static constexpr size_t MAX_PARAMETERS = 32;
    size_t number;
    Parameter parameters[MAX_PARAMETERS];
  };

  struct Command {
    const char *name;
    std::function<void(Console &, CommandLine &)> handler;
  };

  class EditBuffer {
   public:
    static constexpr size_t SIZE = 120;

    EditBuffer() : cursor{0}, end{0} {}
    auto reset() { cursor = end = 0; }

    char buffer[SIZE];
    size_t cursor;
    size_t end;
  };

  Console(const char *banner, const Command *commands);

  auto read_buffer() -> std::pair<uint8_t *, size_t>;
  auto read_done(size_t length) -> void;
  auto write_buffer() -> std::pair<const char *, size_t>;
  auto write_done(size_t length) -> void;

  auto putc(char c) -> bool;
  auto printf(FORMAT format...) -> int;
  auto vprintf(FORMAT format, va_list args) -> int;

 private:
  static constexpr size_t OUTPUT_BUFFER_SIZE = 2048;
  static constexpr size_t RX_BUFFER_SIZE = 64 + 1;

  static char output_buffer_[OUTPUT_BUFFER_SIZE];
  static uint8_t rx_buffer_[RX_BUFFER_SIZE];
  static EditBuffer edit_;
  static Conversion conversion_;
  static CommandLine command_line_;

  auto edit(KeyAction action, char c) -> bool;
  auto tokenise() -> void;
  auto find_command(Parameter &first) -> const Command *;
  auto parse(char c) -> KeyAction;
  auto parse_character(char c) -> KeyAction;
  auto parse_escape_sequence(char c) -> KeyAction;

  const char *banner_;
  const Command *commands_;

  ParseState parse_state_;
  size_t tx_index_;
  size_t tx_sent_;
};

#endif  // IO_CONSOLE_H
