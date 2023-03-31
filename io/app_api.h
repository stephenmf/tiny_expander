#ifndef IO_APP_API_H
#define IO_APP_API_H

#include <cstddef>
#include <cstdint>
#include <utility>

class AppApi {
 public:
  virtual auto init() -> void = 0;
  virtual auto periodic() -> void = 0;

  // External interface api.
  virtual auto read_buffer() -> std::pair<uint8_t*, size_t> = 0;
  virtual auto read_done(size_t length) -> void = 0;
  virtual auto write_buffer() -> std::pair<const char*, size_t> = 0;
  virtual auto write_done(size_t length) -> void = 0;
};

#endif  // IO_APP_API_H
