#ifndef SERVICE_METRICS_H
#define SERVICE_METRICS_H
#include "metric.h"
#include "request_metrics.h"

namespace call_center::core::qs::metrics {

/**
 * @brief Метрики обслуживания прибором.
 *
 * Содержит количество обслуженных запросов, средее и общее время обслуживания.
 */
class ServiceMetrics {
 public:
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  /**
   * @brief Добавить обслуженный запрос.
   * @param cur_service_time время обслуживания запроса
   */
  template <typename Duration_t>
  void AddCompletedService(Duration_t cur_service_time);
  /**
   * @brief Количество обслуженных запросов.
   */
  [[nodiscard]] size_t GetServicedCount() const;
  /**
   * @brief Метрика времени обслуживания.
   */
  [[nodiscard]] const Metric<Duration> &GetServiceTimeMetric() const;
  /**
   * @brief Общее время обслужиания.
   */
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

}  // namespace call_center::core::qs::metrics

#endif  // SERVICE_METRICS_H
