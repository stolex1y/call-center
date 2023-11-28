#ifndef SERVICE_METRICS_H
#define SERVICE_METRICS_H
#include "metric.h"
#include "request_metrics.h"

namespace call_center::qs::metrics {

class ServiceMetrics {
 public:
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  template <typename Duration_t>
  void AddCompletedService(Duration_t cur_service_time);
  [[nodiscard]] size_t GetServicedCount() const;
  [[nodiscard]] const Metric<Duration> &GetServiceTimeMetric() const;
  [[nodiscard]] Duration GetTotalServiceTime() const;

 private:
  Metric<Duration> service_time_{Duration(0)};
  Duration total_service_time_{Duration(0)};
};

template <typename Duration_t>
void ServiceMetrics::AddCompletedService(Duration_t cur_service_time) {
  service_time_.AddValue(cur_service_time);
  total_service_time_ += cur_service_time;
}

}  // namespace call_center::qs::metrics

#endif  // SERVICE_METRICS_H
