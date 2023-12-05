#include "clock_adapter.h"

namespace call_center::core {

std::shared_ptr<const ClockAdapter> ClockAdapter::default_clock =
    std::make_shared<const ClockAdapter>();

ClockAdapter::TimePoint ClockAdapter::Now() const {
  return Clock::now();
}

}  // namespace call_center::core
