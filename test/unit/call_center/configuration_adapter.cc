#include "configuration_adapter.h"

#include <fstream>

#include "call_queue.h"

namespace call_center::test {

ConfigurationAdapter::ConfigurationAdapter(std::shared_ptr<Configuration> configuration)
    : configuration_(std::move(configuration)) {
}

void ConfigurationAdapter::SetOperatorCount(const size_t count) {
  assert(count > 0);
  config_json[OperatorSet::kOperatorCountKey] = count;
}

void ConfigurationAdapter::UpdateConfiguration() const {
  {
    std::ofstream config_file(configuration_->GetFileName());
    assert(config_file);
    config_file << serialize(config_json);
  }

  configuration_->UpdateConfiguration();
}

void ConfigurationAdapter::SetCallQueueCapacity(const size_t capacity) {
  config_json[CallQueue::kCapacityKey] = capacity;
}

void ConfigurationAdapter::SetConfigurationCaching(const bool caching) {
  config_json[Configuration::kCachingKey] = caching;
}

void ConfigurationAdapter::SetOperatorDelay(
    const Operator::DelayDuration min_delay, const Operator::DelayDuration max_delay
) {
  assert(max_delay >= min_delay);
  config_json[Operator::kMinDelayKey] = min_delay.count();
  config_json[Operator::kMaxDelayKey] = max_delay.count();
}

void ConfigurationAdapter::SetOperatorDelay(const Operator::DelayDuration delay) {
  SetOperatorDelay(delay, delay);
}

void ConfigurationAdapter::SetCallMaxWait(const CallDetailedRecord::WaitingDuration max_wait) {
  config_json[CallDetailedRecord::kMaxWaitKey] = max_wait.count();
}

void ConfigurationAdapter::SetMetricsUpdateTime(
    const metrics::QueueingSystemMetrics::MetricsUpdateDuration delay
) {
  config_json[metrics::QueueingSystemMetrics::kMetricsUpdateTimeKey] = delay.count();
}

}  // namespace call_center::test
