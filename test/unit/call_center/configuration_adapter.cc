#include "configuration_adapter.h"

#include <fstream>

#include "call_queue.h"

namespace call_center {

ConfigurationAdapter::ConfigurationAdapter(std::shared_ptr<Configuration> configuration)
    : configuration_(std::move(configuration)) {
}

void ConfigurationAdapter::SetOperatorCount(size_t count) {
  assert(count > 0);
  config_json[OperatorSet::kOperatorCountKey_] = count;
}

void ConfigurationAdapter::UpdateConfiguration() const {
  {
    std::ofstream config_file(configuration_->GetFileName());
    assert(config_file);
    config_file << boost::json::serialize(config_json);
  }

  configuration_->UpdateConfiguration();
}

void ConfigurationAdapter::SetCallQueueCapacity(size_t capacity) {
  config_json[CallQueue::kCapacityKey_] = capacity;
}

void ConfigurationAdapter::SetConfigurationCaching(bool caching) {
  config_json[Configuration::kCachingKey_] = caching;
}

void ConfigurationAdapter::SetOperatorDelay(
    Operator::DelayDuration min_delay, Operator::DelayDuration max_delay
) {
  assert(max_delay >= min_delay);
  config_json[Operator::kMinDelayKey_] = min_delay.count();
  config_json[Operator::kMaxDelayKey_] = max_delay.count();
}

void ConfigurationAdapter::SetOperatorDelay(Operator::DelayDuration delay) {
  SetOperatorDelay(delay, delay);
}

void ConfigurationAdapter::SetCallMaxWait(CallDetailedRecord::WaitingDuration max_wait) {
  config_json[CallDetailedRecord::kMaxWaitKey_] = max_wait.count();
}

void ConfigurationAdapter::SetMetricsUpdateTime(
    qs::metrics::QueueingSystemMetrics::MetricsUpdateDuration delay
) {
  config_json[qs::metrics::QueueingSystemMetrics::kMetricsUpdateTimeKey] = delay.count();
}
}  // namespace call_center
