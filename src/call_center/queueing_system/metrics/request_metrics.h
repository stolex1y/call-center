#ifndef REQUEST_METRICS_H
#define REQUEST_METRICS_H
#include <cassert>
#include <chrono>

namespace call_center::qs::metrics {
class RequestMetrics {
 public:
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  template <typename TimePoint_t>
  explicit RequestMetrics(TimePoint_t arrival_time);

  template <typename TimePoint_t>
  void SetServiceStartTime(TimePoint_t service_start_time);

  template <typename TimePoint_t>
  void SetServiceCompleteTime(TimePoint_t service_complete_time);

  [[nodiscard]] std::optional<Duration> GetWaitTime() const;
  [[nodiscard]] std::optional<Duration> GetTotalTime() const;
  [[nodiscard]] std::optional<Duration> GetServiceTime() const;

 private:
  const TimePoint arrival_time_;
  std::optional<TimePoint> service_start_time_ = std::nullopt;
  std::optional<TimePoint> service_complete_time_ = std::nullopt;
};

template <typename TimePoint_t>
RequestMetrics::RequestMetrics(TimePoint_t arrival_time) : arrival_time_(arrival_time) {
}

template <typename TimePoint_t>
void RequestMetrics::SetServiceStartTime(TimePoint_t service_start_time) {
  assert(!service_start_time_);
  service_start_time_ = service_start_time;
}

template <typename TimePoint_t>
void RequestMetrics::SetServiceCompleteTime(TimePoint_t service_complete_time) {
  assert(service_start_time_ && !service_complete_time_);
  service_complete_time_ = service_complete_time;
}

}  // namespace call_center::qs::metrics

#endif  // REQUEST_METRICS_H
