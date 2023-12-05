#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_

#include <chrono>

namespace call_center::core {

/**
 * @brief Класс-оболочка над классом std::chrono::clock преимущественно для облегчения тестирования.
 */
class ClockAdapter {
 public:
  /**
   * @brief Результирующий тип часов: std::chrono::utc_clock.
   */
  using Clock = std::chrono::utc_clock;
  /**
   * @brief Результирующий временной интервал: std::chrono::nanoseconds.
   */
  using Duration = std::chrono::nanoseconds;
  /**
   * @brief Временная точка, основанная на часах типа @link Clock @endlink.
   */
  using TimePoint = std::chrono::time_point<Clock>;

  /**
   * @brief Часы по умолчанию (см. @link Clock @endlink).
   */
  static std::shared_ptr<const ClockAdapter> default_clock;

  virtual ~ClockAdapter() = default;

  /**
   * @brief Текущее время.
   */
  [[nodiscard]] virtual TimePoint Now() const;
};

}  // namespace call_center::core

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_
