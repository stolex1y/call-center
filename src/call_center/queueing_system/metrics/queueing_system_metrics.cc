#include "queueing_system_metrics.h"

#include <mutex>
#include <ranges>
#include <unordered_set>

#include "service_metrics.h"

namespace call_center::qs::metrics {

std::shared_ptr<QueueingSystemMetrics> QueueingSystemMetrics::Create(
    std::shared_ptr<core::TaskManager> task_manager,
    std::shared_ptr<Configuration> configuration,
    const log::LoggerProvider& logger_provider,
    std::shared_ptr<const core::ClockAdapter> clock
) {
  return std::shared_ptr<QueueingSystemMetrics>(new QueueingSystemMetrics(
    std::move(task_manager),
    std::move(configuration),
    logger_provider,
    std::move(clock)
  ));
}

QueueingSystemMetrics::QueueingSystemMetrics(
    std::shared_ptr<core::TaskManager> task_manager,
    std::shared_ptr<Configuration> configuration,
    const log::LoggerProvider& logger_provider,
    std::shared_ptr<const core::ClockAdapter> clock
)
  : clock_(std::move(clock)),
    configuration_(std::move(configuration)),
    task_manager_(std::move(task_manager)),
    logger_(logger_provider.Get("QueueingSystemMetrics")) {
}

void QueueingSystemMetrics::Start() {
  if (started_.test_and_set()) {
    return;
  }
  logger_->Info() << "Start periodic metrics updating";
  Reset();
  ScheduleUpdatePeriodicMetrics();
}

void QueueingSystemMetrics::Reset() {
  logger_->Info() << "Reset recording start time";
  recording_start_time_ = std::chrono::time_point_cast<Duration>(clock_->Now());
}

void QueueingSystemMetrics::Stop() {
  if (!started_.test()) {
    return;
  }
  logger_->Info() << "Stop periodic metrics updating";
  started_.clear();
}

void QueueingSystemMetrics::RecordRequestArrival(const RequestPtr &request) {
  assert(request->WasArrived());
  std::lock_guard lock(queue_mutex_);
  ++arrival_count_;
  ++current_in_system_count_;
  ++current_queue_size_;
  UpdateAvgTimeBetweenRequests(*request->GetArrivalTime());
}

void QueueingSystemMetrics::RecordServiceStart(const RequestPtr &request) {
  assert(request->GetWaitTime());
  std::lock_guard lock(queue_mutex_);
  --current_queue_size_;
  wait_time_.AddValue(*request->GetWaitTime());
}

void QueueingSystemMetrics::RecordServiceComplete(
    const RequestPtr &request, const ServerPtr &server
) {
  assert(request->WasServiced());
  std::lock_guard service_lock(service_mutex_);
  assert(servers_metrics_.contains(server));
  GetServerMetrics(server).AddCompletedService(*request->GetServiceTime());
  {
    std::lock_guard queue_lock(queue_mutex_);
    --current_in_system_count_;
  }
}

void QueueingSystemMetrics::RecordRequestDropout(const RequestPtr &request) {
  assert(request->WasFinished());
  {
    std::lock_guard queue_lock(queue_mutex_);
    --current_queue_size_;
    --current_in_system_count_;
  }
  std::lock_guard dropout_lock(dropout_mutex_);
  ++dropout_count_;
  refused_wait_time_.AddValue(*request->GetWaitTime());
}

void QueueingSystemMetrics::UpdatePeriodicMetrics() {
  {
    std::lock_guard queue_lock(queue_mutex_);
    queue_size_.AddValue(current_queue_size_);
    in_system_count_.AddValue(current_in_system_count_);
  }
  {
    std::lock_guard service_lock(service_mutex_);
    busy_server_count_.AddValue(GetCurrentBusyServerCount());
  }
}

void QueueingSystemMetrics::ScheduleUpdatePeriodicMetrics() {
  if (!started_.test()) {
    return;
  }
  UpdateMetricsUpdateTime();
  task_manager_->PostTaskDelayed(
      MetricsUpdateDuration(metrics_update_time_),
      [qsm = shared_from_this()]() {
        qsm->UpdatePeriodicMetrics();
        qsm->ScheduleUpdatePeriodicMetrics();
      }
  );
}

size_t QueueingSystemMetrics::GetCurrentBusyServerCount() const {
  size_t count = 0;
  for (const auto &server : std::views::keys(servers_metrics_)) {
    if (server->IsBusy()) {
      ++count;
    }
  }
  return count;
}

void QueueingSystemMetrics::UpdateAvgTimeBetweenRequests(TimePoint last_arrival_time_) {
  if (prev_arrival_) {
    const Duration cur_duration = last_arrival_time_ - *prev_arrival_;
    time_between_requests_.AddValue(cur_duration);
  }
  prev_arrival_ = last_arrival_time_;
}

ServiceMetrics &QueueingSystemMetrics::GetServerMetrics(const ServerPtr &server) {
  return servers_metrics_.find(server)->second;
}

size_t QueueingSystemMetrics::GetServicedCount() const {
  size_t count = 0;
  for (const auto &metrics : std::views::values(servers_metrics_)) {
    count += metrics.GetServicedCount();
  }
  return count;
}

void QueueingSystemMetrics::UpdateMetricsUpdateTime() {
  metrics_update_time_ = configuration_->GetProperty(kMetricsUpdateTimeKey, metrics_update_time_);
}

Metric<QueueingSystemMetrics::Duration> QueueingSystemMetrics::GetWaitTimeMetric() const {
  std::shared_lock lock(queue_mutex_);
  return wait_time_;
}

Metric<size_t, double> QueueingSystemMetrics::GetQueueSizeMetric() const {
  std::shared_lock lock(queue_mutex_);
  return queue_size_;
}

Metric<size_t, double> QueueingSystemMetrics::GetBusyServerCountMetric() const {
  std::shared_lock lock(service_mutex_);
  return busy_server_count_;
}

double QueueingSystemMetrics::GetServiceLoadInErlang() const {
  std::shared_lock lock(service_mutex_);
  Duration total_service_time(0);
  for (const auto &service_metrics : std::views::values(servers_metrics_)) {
    total_service_time += service_metrics.GetTotalServiceTime();
  }
  const auto duration_since_start =
      std::chrono::duration_cast<Duration>(clock_->Now() - recording_start_time_.load());
  return static_cast<double>(total_service_time.count()) /
         static_cast<double>(duration_since_start.count());
}

Metric<QueueingSystemMetrics::Duration> QueueingSystemMetrics::GetTimeBetweenRequestsMetric(
) const {
  std::shared_lock lock(queue_mutex_);
  return time_between_requests_;
}

size_t QueueingSystemMetrics::GetDropoutCount() const {
  std::shared_lock lock(dropout_mutex_);
  return dropout_count_;
}

Metric<QueueingSystemMetrics::Duration> QueueingSystemMetrics::GetRefusedWaitTimeMetric() const {
  std::shared_lock lock(dropout_mutex_);
  return refused_wait_time_;
}

QueueingSystemMetrics::Duration QueueingSystemMetrics::GetAverageServiceTime() const {
  std::shared_lock lock(service_mutex_);
  Duration avg_service_time_{0};
  for (const auto& service_metrics : std::views::values(servers_metrics_)) {
    avg_service_time_ += service_metrics.GetServiceTimeMetric().GetAvg();
  }
  return avg_service_time_;
}

Metric<size_t, double> QueueingSystemMetrics::GetRequestCountInSystemMetric() const {
  std::shared_lock lock(queue_mutex_);
  return in_system_count_;
}

double QueueingSystemMetrics::GetProbabilityOfLoss() const {
  std::shared_lock service_lock(service_mutex_);
  const size_t serviced_count = GetServicedCount();
  service_lock.unlock();
  std::shared_lock queue_lock(queue_mutex_);
  return static_cast<double>(serviced_count) / arrival_count_;
}

bool QueueingSystemMetrics::ServerEquals::operator()(
  const ServerPtr& first,
  const ServerPtr& second
) const {
  if ((first == nullptr) ^ (second == nullptr))
    return false;

  if (first == nullptr)
    return true;

  return first->GetId() == second->GetId();
}

size_t QueueingSystemMetrics::ServerHash::operator()(const ServerPtr& server) const {
  return server->hash();
}

}  // namespace call_center::qs::metrics
