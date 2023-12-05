#ifndef CALL_CENTER_SRC_CALL_CENTER_QUEUEING_SYSTEM_METRICS_H_
#define CALL_CENTER_SRC_CALL_CENTER_QUEUEING_SYSTEM_METRICS_H_

#include <atomic>
#include <chrono>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "core/clock_adapter.h"
#include "core/queueing_system/request.h"
#include "core/queueing_system/server.h"
#include "core/tasks/task_manager.h"
#include "metric.h"
#include "operator.h"
#include "service_metrics.h"

/// Метрики для СМО.
namespace call_center::core::qs::metrics {

/**
 * @brief Класс для подсчета различных метрик для системы массового обслуживания.
 */
class QueueingSystemMetrics : public std::enable_shared_from_this<QueueingSystemMetrics> {
 public:
  using RequestPtr = std::shared_ptr<const Request>;
  using ServerPtr = std::shared_ptr<const Server>;
  /// Основной временной промежуток, используемый в метриках.
  using Duration = Request::Duration;
  /// Основной тип временной точки, используемый в метриках.
  using TimePoint = Request::TimePoint;
  /// Временной промежуток обновления метрик.
  using MetricsUpdateDuration = std::chrono::seconds;

  /// Ключ в конфигурации, соответствующий значению времени обновления метрик в секундах.
  static constexpr auto kMetricsUpdateTimeKey = "metrics_update_time";

  static std::shared_ptr<QueueingSystemMetrics> Create(
      std::shared_ptr<tasks::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const log::LoggerProvider &logger_provider,
      std::shared_ptr<const ClockAdapter> clock = ClockAdapter::default_clock
  );

  QueueingSystemMetrics(const QueueingSystemMetrics &other) = delete;
  QueueingSystemMetrics &operator=(const QueueingSystemMetrics &other) = delete;

  /**
   * @brief Запустить периодическое обновление метрик.
   */
  void Start();
  /**
   * @brief Сбросить все метрики.
   */
  void Reset();
  /**
   * @brief Остановить обновление метрик.
   */
  void Stop();

  /**
   * @brief Добавить обслуживающие приборы для отслеживания их статуса.
   * @tparam ServerImplSet множество приборов
   */
  template <typename ServerImplSet>
  void SetServers(const ServerImplSet &server_impls);
  /**
   * @brief Зафиксировать получение нового запроса.
   */
  void RecordRequestArrival(const RequestPtr &request);
  /**
   * @brief Зафиксировать начало обработки запроса.
   */
  void RecordServiceStart(const RequestPtr &request);
  /**
   * @brief Зафиксировать завершение обработки запроса указанным прибором.
   */
  void RecordServiceComplete(const RequestPtr &request, const ServerPtr &server);
  /**
   * @brief Зафиксировать отклоненный запрос.
   */
  void RecordRequestDropout(const RequestPtr &request);

  /**
   * @brief Получить метрики по времени ожидания в очереди.
   */
  [[nodiscard]] Metric<Duration> GetWaitTimeMetric() const;
  /**
   * @brief Получить метрики по размеру очереди.
   */
  [[nodiscard]] Metric<size_t, double> GetQueueSizeMetric() const;
  /**
   * @brief Получить метрики по количеству обслуживающих приборов.
   */
  [[nodiscard]] Metric<size_t, double> GetBusyServerCountMetric() const;
  /**
   * @brief Получить обслуженную нагрузку в Эрлангах (E) с начала записи.
   */
  [[nodiscard]] double GetServiceLoadInErlang() const;
  /**
   * @brief Получить метрики по среднему времени между запросами.
   */
  [[nodiscard]] Metric<Duration> GetTimeBetweenRequestsMetric() const;
  /**
   * @brief Получить количество отклоненных запросов.
   */
  [[nodiscard]] size_t GetDropoutCount() const;
  /**
   * @brief Получить метрики по времени ожидания отклоненных запросов.
   */
  [[nodiscard]] Metric<Duration> GetRefusedWaitTimeMetric() const;
  /**
   * @brief Получить среднее время обслуживания среди всех приборов.
   */
  [[nodiscard]] Duration GetAverageServiceTime() const;
  /**
   * @brief Получить метрики по количеству запросов, одновременно находящихся в системе.
   */
  [[nodiscard]] Metric<size_t, double> GetRequestCountInSystemMetric() const;
  /**
   * @brief Получить вероятность потери запроса.
   */
  [[nodiscard]] double GetProbabilityOfLoss() const;
  /**
   * @brief Получить количество обслуженных запросов.
   */
  [[nodiscard]] size_t GetServicedCount() const;
  /**
   * @brief Получить количество принятых запросов.
   */
  [[nodiscard]] size_t GetArrivalCount() const;

 private:
  struct ServerEquals {
    bool operator()(const ServerPtr &first, const ServerPtr &second) const;
  };

  struct ServerHash {
    size_t operator()(const ServerPtr &server) const;
  };

  static constexpr uint64_t kDefaultMetricsUpdateTime = 10;

  std::shared_ptr<const ClockAdapter> clock_;

  std::atomic_flag started_ = false;
  const std::shared_ptr<Configuration> configuration_;
  const std::shared_ptr<tasks::TaskManager> task_manager_;
  const std::unique_ptr<log::Logger> logger_;
  uint64_t metrics_update_time_ = kDefaultMetricsUpdateTime;
  std::atomic<TimePoint> recording_start_time_ = TimePoint(Duration(0));

  Metric<Duration> time_between_requests_{Duration(0)};
  std::optional<TimePoint> prev_arrival_;
  size_t arrival_count_ = 0;
  Metric<Duration> wait_time_{Duration(0)};
  Metric<size_t, double> queue_size_{0};
  Metric<size_t, double> in_system_count_{0};
  size_t current_queue_size_ = 0;
  size_t current_in_system_count_ = 0;
  mutable std::shared_mutex queue_mutex_;

  std::size_t dropout_count_ = 0;
  Metric<Duration> refused_wait_time_{Duration(0)};
  mutable std::shared_mutex dropout_mutex_;

  std::unordered_map<ServerPtr, ServiceMetrics, ServerHash, ServerEquals> servers_metrics_;
  Metric<size_t, double> busy_server_count_{0};
  mutable std::shared_mutex service_mutex_;

  QueueingSystemMetrics(
      std::shared_ptr<tasks::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const log::LoggerProvider &logger_provider,
      std::shared_ptr<const ClockAdapter> clock
  );

  /**
   * @brief Обновить периодические метрики.
   */
  void UpdatePeriodicMetrics();
  /**
   * @brief Запланировать очередное периодическое обновление метрик.
   */
  void ScheduleUpdatePeriodicMetrics();
  /**
   * @brief Получить текущее количество занятых приборов.
   */
  [[nodiscard]] size_t GetCurrentBusyServerCount() const;
  /**
   * @brief Обновить среднее время между запросами, используя время получения нового запрсоа.
   */
  void UpdateAvgTimeBetweenRequests(TimePoint last_arrival_time_);
  /**
   * @brief Получить метрики для заданного прибора.
   */
  ServiceMetrics &GetServerMetrics(const ServerPtr &server);
  /**
   * @brief Обновить интервал обновления метрик.
   */
  void UpdateMetricsUpdateTime();
};

template <typename ServerImplSet>
void QueueingSystemMetrics::SetServers(const ServerImplSet &server_impls) {
  const std::unordered_set<ServerPtr, ServerHash, ServerEquals> servers(
      server_impls.begin(), server_impls.end()
  );
  std::lock_guard lock(service_mutex_);
  erase_if(servers_metrics_, [&servers](const auto &p) {
    return !servers.contains(p.first);
  });
  for (const auto &server : servers) {
    if (!servers_metrics_.contains(server)) {
      servers_metrics_.emplace(server, ServiceMetrics{});
    }
  }
}

}  // namespace call_center::core::qs::metrics

#endif  // CALL_CENTER_SRC_CALL_CENTER_QUEUEING_SYSTEM_METRICS_H_
