#include "request_metrics.h"

namespace call_center::qs::metrics {

std::optional<RequestMetrics::Duration> RequestMetrics::GetServiceTime() const {
  if (!service_complete_time_) {
    return std::nullopt;
  }
  return *service_complete_time_ - *service_start_time_;
}

std::optional<RequestMetrics::Duration> RequestMetrics::GetWaitTime() const {
  if (!service_start_time_) {
    return std::nullopt;
  }
  return *service_start_time_ - arrival_time_;
}

std::optional<RequestMetrics::Duration> RequestMetrics::GetTotalTime() const {
  if (!service_complete_time_) {
    return std::nullopt;
  }
  return *service_complete_time_ - arrival_time_;
}

}  // namespace call_center::qs::metrics
