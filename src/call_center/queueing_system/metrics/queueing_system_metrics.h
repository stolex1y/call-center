#ifndef CALL_CENTER_SRC_CALL_CENTER_QUEUEING_SYSTEM_METRICS_H_
#define CALL_CENTER_SRC_CALL_CENTER_QUEUEING_SYSTEM_METRICS_H_

#include <atomic>
#include <chrono>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "core/clock_adapter.h"
#include "core/task_manager.h"
#include "metric.h"
#include "operator.h"
#include "queueing_system/request.h"
#include "queueing_system/server.h"
#include "service_metrics.h"

namespace call_center::qs::metrics {

class QueueingSystemMetrics : public std::enable_shared_from_this<QueueingSystemMetrics> {
 public:
  using RequestPtr = std::shared_ptr<const Request>;
  using ServerPtr = std::shared_ptr<const Server>;
  using Duration = Request::Duration;
  using TimePoint = Request::TimePoint;
  using MetricsUpdateDuration = std::chrono::seconds;

  static constexpr auto kMetricsUpdateTimeKey = "metrics_update_time";

  static std::shared_ptr<QueueingSystemMetrics> Create(
      std::shared_ptr<core::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const log::LoggerProvider &logger_provider,
      std::shared_ptr<const core::ClockAdapter> clock = core::ClockAdapter::default_clock
  );

  QueueingSystemMetrics(const QueueingSystemMetrics &other) = delete;
  QueueingSystemMetrics &operator=(const QueueingSystemMetrics &other) = delete;

  void Start();
  void Reset();
  void Stop();

  template <typename ServerImplSet>
  void SetServers(const ServerImplSet &server_impls);
  void RecordRequestArrival(const RequestPtr &request);
  void RecordServiceStart(const RequestPtr &request);
  void RecordServiceComplete(const RequestPtr &request, const ServerPtr &server);
  void RecordRequestDropout(const RequestPtr &request);

  [[nodiscard]] Metric<Duration> GetWaitTimeMetric() const;
  [[nodiscard]] Metric<size_t, double> GetQueueSizeMetric() const;
  [[nodiscard]] Metric<size_t, double> GetBusyServerCountMetric() const;
  [[nodiscard]] double GetServiceLoadInErlang() const;
  [[nodiscard]] Metric<Duration> GetTimeBetweenRequestsMetric() const;
  [[nodiscard]] size_t GetDropoutCount() const;
  [[nodiscard]] Metric<Duration> GetRefusedWaitTimeMetric() const;
  [[nodiscard]] Duration GetAverageServiceTime() const;
  [[nodiscard]] Metric<size_t, double> GetRequestCountInSystemMetric() const;
  [[nodiscard]] double GetProbabilityOfLoss() const;

 private:
  struct ServerEquals {
    bool operator()(const ServerPtr &first, const ServerPtr &second) const;
  };

  struct ServerHash {
    size_t operator()(const ServerPtr &server) const;
  };

  static constexpr uint64_t kDefaultMetricsUpdateTime = 10;

  std::shared_ptr<const core::ClockAdapter> clock_;

  std::atomic_flag started_ = false;
  const std::shared_ptr<Configuration> configuration_;
  const std::shared_ptr<core::TaskManager> task_manager_;
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
      std::shared_ptr<core::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const log::LoggerProvider &logger_provider,
      std::shared_ptr<const core::ClockAdapter> clock
  );

  void UpdatePeriodicMetrics();
  void ScheduleUpdatePeriodicMetrics();
  [[nodiscard]] size_t GetCurrentBusyServerCount() const;
  void UpdateAvgTimeBetweenRequests(TimePoint last_arrival_time_);
  ServiceMetrics &GetServerMetrics(const ServerPtr &server);
  [[nodiscard]] size_t GetServicedCount() const;
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

}  // namespace call_center::qs::metrics

#endif  // CALL_CENTER_SRC_CALL_CENTER_QUEUEING_SYSTEM_METRICS_H_
