#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_

#include <boost/json.hpp>

#include "core/utils/numbers.h"
#include "queueing_system/metrics/metric.h"
#include "queueing_system/metrics/queueing_system_metrics.h"

namespace call_center::repository {
namespace json = boost::json;

using namespace qs::metrics;
using namespace std::chrono;
using namespace core::utils::numbers;

struct MetricsReponseDto {
  using Duration = QueueingSystemMetrics::Duration;
  using TimePoint = QueueingSystemMetrics::TimePoint;

  Metric<Duration> wait_time_metric;
  Metric<size_t, double> queue_size_metric;
  Metric<size_t, double> busy_operators_count_metric;
  double service_load_in_erlang;

  MetricsReponseDto(
      Metric<Duration> wait_time_metric,
      Metric<size_t, double> queue_size_metric,
      Metric<size_t, double> busy_operators_count_metric,
      double service_load_in_erlang
  );

  friend void tag_invoke(
      const json::value_from_tag &, json::value &json, const MetricsReponseDto &call_response
  );

  template <typename Metric>
  static json::value MetricToJson(Metric metric);
};

template <typename Metric>
json::value MetricsReponseDto::MetricToJson(Metric metric) {
  return json::object{
      {"min", metric.GetMin()}, {"max", metric.GetMax()}, {"avg", round(metric.GetAvg(), 1e-3)}};
}

template <>
inline json::value MetricsReponseDto::MetricToJson<Metric<MetricsReponseDto::Duration>>(
    Metric<Duration> metric
) {
  return json::object{
      {"min", floor<seconds>(metric.GetMin()).count()},
      {"max", floor<seconds>(metric.GetMax()).count()},
      {"avg", round(floor<duration<double>>(metric.GetAvg()).count(), 1e-3)}};
}

}  // namespace call_center::repository

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_CALL_RESPONSE_DTO_H_
