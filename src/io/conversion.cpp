#include "io/conversion.h"

// #include "app/hal/util.h"

auto Conversion::to_signed_int(const char *start, unsigned base) -> int {
  // hal_error_handler(__FILE__, __LINE__, "Implement me!");
  return 0;
}

auto Conversion::to_unsigned_int(const char *start, unsigned base) -> unsigned {
  // hal_error_handler(__FILE__, __LINE__, "Implement me!");
  return 0;
}

auto Conversion::to_double(const char *start) -> double {
  // hal_error_handler(__FILE__, __LINE__, "Implement me!");
  return 0.0;
}

auto Conversion::reset() -> void {
  left_justified_ = false;
  output_sign_ = false;
  pad_positive_ = false;
  alternate_ = false;
  upper_case_ = false;
  force_exponent_ = false;
  dynamic_ = false;
  precision_ = 1;
  base_ = 0;
  width_ = 1;
  fill_ = ' ';
}

auto Conversion::parse(FORMAT &format) -> Type {
  reset();
  auto c = *format++;
  if (c == '\0') return Type::UNKNOWN;
  if (c == '-') {
    left_justified_ = true;
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  if (c == '+') {
    output_sign_ = true;
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  if (c == ' ') {
    pad_positive_ = true;
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  if (c == '#') {
    alternate_ = true;
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  if (c == '0') {
    fill_ = '0';
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  bool is_long = false;
  if (c == 'l' || c == 'L') {
    is_long = true;
    c = *format++;
  } else if (c == 'h' || c == 'j' || c == 'z' || c == 't') {
    // ignore
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  bool is_long_long = false;
  if (c == 'l') {
    is_long = true;
    c = *format++;
  } else if (c == 'h') {
    // ignore
    c = *format++;
  }
  if (c == '\0') return Type::UNKNOWN;
  int width = -1;
  if (c > '0' && c <= '9') {
    width = 0;
    while (c > '0' && c <= '9') {
      width = (width * 10) + (c - '0');
      c = *format++;
    }
  }
  if (c == '\0') return Type::UNKNOWN;
  int precision = -1;
  if (c == '.') {
    precision = 0;
    c = *format;
    while (c > '0' && c <= '9') {
      precision = (precision * 10) + (c - '0');
      c = *format++;
    }
  }
  bool is_unsigned = true;
  if (c == '\0') return Type::UNKNOWN;
  if (c == '%')
    return Type::PERCENT;
  else if (c == 'c')
    return Type::CHARACTER;
  else if (c == 's') {
    if (width > 0)
      width_ = width;
    else
      width_ = 0;
    if (precision > 0)
      precision_ = precision;
    else
      precision_ = 0;
    return Type::STRING;
    // Integer conversions
  } else if (c == 'p') {
    reset();
    base_ = 16;
    width_ = 8;
    upper_case_ = true;
    return Type::POINTER;
  } else if (c == 'd' || c == 'i') {
    is_unsigned = false;
    base_ = 10;
  } else if (c == 'o')
    base_ = 8;
  else if (c == 'b')
    base_ = 2;
  else if (c == 'X') {
    upper_case_ = true;
    base_ = 16;
  } else if (c == 'x')
    base_ = 16;
  if (base_ != 0) {
    if (width < 0)
      width_ = 1;
    else
      width_ = width;
    if (precision < 0)
      precision_ = 1;
    else
      precision_ = precision;
    if (is_unsigned) {
      if (is_long_long) return Type::LONG_LONG_UNSIGNED_INT;
      if (is_long) return Type::LONG_UNSIGNED_INT;
      return Type::UNSIGNED_INT;
    } else {
      if (is_long_long) return Type::LONG_LONG_SIGNED_INT;
      if (is_long) return Type::LONG_SIGNED_INT;
      return Type::SIGNED_INT;
    }
  }
  // Floating point conversions.
  base_ = 10;
  if (width < 0)
    width_ = 1;
  else
    width_ = width;
  if (precision < 0)
    precision_ = 6;
  else
    precision_ = precision;
  if (c == 'f')
    return Type::DOUBLE;
  else if (c == 'F') {
    upper_case_ = true;
    return Type::DOUBLE;
  } else if (c == 'e') {
    force_exponent_ = true;
    return Type::DOUBLE;
  } else if (c == 'E') {
    force_exponent_ = true;
    upper_case_ = true;
    return Type::DOUBLE;
  } else if (c == 'a') {
    base_ = 16;
    force_exponent_ = true;
    return Type::DOUBLE;
  } else if (c == 'A') {
    base_ = 16;
    force_exponent_ = true;
    upper_case_ = true;
    return Type::DOUBLE;
  } else if (c == 'g') {
    dynamic_ = true;
    return Type::DOUBLE;
  } else if (c == 'G') {
    dynamic_ = true;
    upper_case_ = true;
    return Type::DOUBLE;
  }
  return Type::UNKNOWN;
}

auto Conversion::from_character(long long value) -> const char * {
  buffer_[0] = value;
  buffer_[1] = '\0';
  return buffer_;
}

auto Conversion::from_signed_int(long long value) -> const char * {
  if (value < 0) return from_int_(true, -value);
  return from_int_(false, value);
}

auto Conversion::from_unsigned_int(unsigned long long value) -> const char * {
  return from_int_(false, value);
}

auto Conversion::from_double(double value) -> const char * {
  // hal_error_handler(__FILE__, __LINE__, "Implement me!");
  return nullptr;
}

auto Conversion::from_string(const char *str) -> const char * {
  size_t index = 0;
  auto end = sizeof(buffer_) - 1;
  if (precision_ > 0 && precision_ < end) end = precision_;
  while (index < end && *str != 0) buffer_[index++] = *str++;
  if (left_justified_) {
    while (index < width_) buffer_[index++] = ' ';
  } else if (index < width_) {
    auto scan = width_;
    while (index > 0) buffer_[--scan] = buffer_[--index];
    while (scan > 0) buffer_[--scan] = ' ';
    index = width_;
  }
  buffer_[index] = '\0';
  return buffer_;
}

auto Conversion::from_int_(bool negative, unsigned long long value) -> const
    char * {
  size_t index = 0;
  if (value == 0 && precision_ == 0) {
    buffer_[index] = '\0';
    return buffer_;
  }
  // Output the coversion backwards will sort out later.
  if (upper_case_)
    index = convert_upper_(index, value, base_);
  else
    index = convert_lower_(index, value, base_);
  while (index < precision_) buffer_[index++] = '0';
  if (negative)
    buffer_[index++] = '-';
  else if (pad_positive_)
    buffer_[index++] = fill_;
  if (left_justified_) {
    // Reverse it.
    auto flip = 0U;
    auto flop = index - 1;
    while (flip < flop) {
      auto tmp = buffer_[flip];
      buffer_[flip++] = buffer_[flop];
      buffer_[flop--] = tmp;
    }
    while (index < width_) buffer_[index++] = fill_;
  } else {
    while (index < width_) buffer_[index++] = fill_;
    // Reverse it.
    auto flip = 0U;
    auto flop = index - 1;
    while (flip < flop) {
      auto tmp = buffer_[flip];
      buffer_[flip++] = buffer_[flop];
      buffer_[flop--] = tmp;
    }
  }
  buffer_[index] = '\0';
  return buffer_;
}

auto Conversion::convert_lower_(size_t index, unsigned long long value,
                                unsigned base) -> int {
  while (value != 0) {
    auto c = value % base;
    value = value / base;
    if (c > 9)
      buffer_[index++] = c + ('a' - 10);
    else
      buffer_[index++] = c + '0';
  }
  return index;
}

auto Conversion::convert_upper_(size_t index, unsigned long long value,
                                unsigned base) -> int {
  while (value != 0) {
    auto c = value % base;
    value = value / base;
    if (c > 9)
      buffer_[index++] = c + ('A' - 10);
    else
      buffer_[index++] = c + '0';
  }
  return index;
}
