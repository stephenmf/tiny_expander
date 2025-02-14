#ifndef IO_CONVERSION_H
#define IO_CONVERSION_H

#include <cstddef>
#include <cstdint>

class Conversion {
  using FORMAT = const char *;

 public:
  enum class Type : uint8_t {
    UNKNOWN,
    PERCENT,
    CHARACTER,
    SIGNED_INT,
    UNSIGNED_INT,
    LONG_SIGNED_INT,
    LONG_UNSIGNED_INT,
    LONG_LONG_SIGNED_INT,
    LONG_LONG_UNSIGNED_INT,
    DOUBLE,
    STRING,
    POINTER
  };
  static auto to_signed_int(const char *start, unsigned base = 10) -> int;
  static auto to_unsigned_int(const char *start,
                              unsigned base = 10) -> unsigned;
  static auto to_double(const char *start) -> double;

  auto reset() -> void;
  auto parse(FORMAT &format) -> Type;
  auto from_character(long long value) -> const char *;
  auto from_signed_int(long long value) -> const char *;
  auto from_unsigned_int(unsigned long long value) -> const char *;
  auto from_double(double value) -> const char *;
  auto from_string(const char *str) -> const char *;

 private:
  static constexpr size_t CONVERSION_BUFFER_SIZE = 120;

  auto from_int_(bool negative, unsigned long long value) -> const char *;
  auto convert_lower_(size_t index, unsigned long long value,
                      unsigned base) -> int;
  auto convert_upper_(size_t index, unsigned long long value,
                      unsigned base) -> int;

  unsigned left_justified_ : 1;
  unsigned output_sign_ : 1;
  unsigned pad_positive_ : 1;
  unsigned alternate_ : 1;
  unsigned upper_case_ : 1;
  unsigned force_exponent_ : 1;
  unsigned dynamic_ : 1;
  unsigned precision_ : 4;
  unsigned base_ : 5;
  unsigned width_ : 6;
  unsigned fill_ : 8;
  char buffer_[CONVERSION_BUFFER_SIZE];
};

#endif  // IO_CONVERSION_Honce
