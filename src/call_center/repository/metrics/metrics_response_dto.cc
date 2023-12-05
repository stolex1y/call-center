#include "metrics_response_dto.h"

namespace call_center::repository {

void tag_invoke(
    const json::value_from_tag &, json::value &json, const MetricsReponseDto &metrics_response
) {
  json = json::object{
      {"wait_time", MetricsReponseDto::MetricToJson(metrics_response.wait_time_metric)},
      {"queue_size", MetricsReponseDto::MetricToJson(metrics_response.queue_size_metric)},
      {"busy_operators_count",
       MetricsReponseDto::MetricToJson(metrics_response.busy_operators_count_metric)},
      {"service_load_erlang", metrics_response.service_load_in_erlang}};
}

MetricsReponseDto::MetricsReponseDto(
    const Metric<Duration> &wait_time_metric,
    const Metric<size_t, double> &queue_size_metric,
    const Metric<size_t, double> &busy_operators_count_metric,
    const double service_load_in_erlang
)
    : wait_time_metric(wait_time_metric),
      queue_size_metric(queue_size_metric),
      busy_operators_count_metric(busy_operators_count_metric),
      service_load_in_erlang(service_load_in_erlang) {
}

}  // namespace call_center::repository
