#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_

#include <chrono>

namespace call_center {

class ClockInterface {
 public:
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  [[nodiscard]] virtual TimePoint Now() const = 0;
};

}  // namespace call_center

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_CLOCK_INTERFACE_H_
