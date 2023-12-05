#ifndef NUMBERS_H
#define NUMBERS_H
#include <format>
#include <string>

/// Вспомогательные классы для работы с числами.
namespace call_center::core::utils::numbers {

/**
 * @brief Пребразовать вещественное число в строку.
 * @param precision кол-во знаков после запятой
 */
inline std::string to_string(double d, int precision) {
  return std::format("{:.{}}", d, precision);
}

/**
 * @brief Округлить число до заданной точности.
 * @param d число
 * @param precision точность (например, 0.001)
 */
inline double round(double d, double precision) {
  return std::round(d / precision) * precision;
}

}  // namespace call_center::core::utils::numbers

#endif  // NUMBERS_H
