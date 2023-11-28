#include "service_metrics.h"

namespace call_center::qs::metrics {

size_t ServiceMetrics::GetServicedCount() const {
  return service_time_.GetCount();
}

const Metric<ServiceMetrics::Duration> &ServiceMetrics::GetServiceTimeMetric() const {
  return service_time_;
}

ServiceMetrics::Duration ServiceMetrics::GetTotalServiceTime() const {
  return total_service_time_;
}

}  // namespace call_center::qs::metrics
