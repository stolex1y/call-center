#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_CLOCK_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_CLOCK_H_

#include <chrono>
#include <shared_mutex>

#include "core/clock_adapter.h"

namespace call_center::test {

/**
 * @brief Виртаульные часы.
 *
 * Данные часы идут только за счет переводов вперед на необходимое время.
 */
class FakeClock : public core::ClockAdapter {
 public:
  void AdvanceTo(TimePoint target_time);
  void AdvanceOn(Duration duration);
  [[nodiscard]] TimePoint Now() const override;
  [[nodiscard]] Duration GetRelativeTime() const;

 private:
  Duration relative_{0};
  TimePoint absolute_{std::chrono::floor<Duration>(Clock::now())};
  mutable std::shared_mutex mutex_;
};

}  // namespace call_center::test

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_CLOCK_H_
