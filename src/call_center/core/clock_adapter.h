#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_

#include <chrono>

namespace call_center::core {

class ClockAdapter {
 public:
  using Clock_t = std::chrono::utc_clock;
  using Duration = std::chrono::nanoseconds;
  using TimePoint = std::chrono::time_point<Clock_t>;

  static std::shared_ptr<const ClockAdapter> default_clock;

  virtual ~ClockAdapter() = default;

  [[nodiscard]] virtual TimePoint Now() const;
};

}  // namespace call_center

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_
