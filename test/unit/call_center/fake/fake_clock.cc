#include "fake_clock.h"

namespace call_center::test {

void FakeClock::AdvanceTo(const TimePoint target_time) {
  std::lock_guard lock(mutex_);
  if (target_time > absolute_) {
    relative_ += target_time - absolute_;
    absolute_ = target_time;
  }
}

FakeClock::TimePoint FakeClock::Now() const {
  std::shared_lock lock(mutex_);
  return absolute_;
}

FakeClock::Duration FakeClock::GetRelativeTime() const {
  std::shared_lock lock(mutex_);
  return relative_;
}

void FakeClock::AdvanceOn(Duration duration) {
  AdvanceTo(Now() + duration);
}

}  // namespace call_center::test