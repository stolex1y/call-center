#ifndef NUMBERS_H
#define NUMBERS_H
#include <format>
#include <string>

namespace call_center::core::utils::numbers {

inline std::string to_string(double d, int precision) {
  return std::format("{:.{}}", d, precision);
}

inline double round(double d, double precision) {
  return std::round(d / precision) * precision;
}

}  // namespace call_center::core::utils::numbers

#endif  // NUMBERS_H
