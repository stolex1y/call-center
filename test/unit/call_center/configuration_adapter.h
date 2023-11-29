#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_CONFIGURATION_ADAPTER_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_CONFIGURATION_ADAPTER_H_

#include <boost/json.hpp>
#include <memory>

#include "call_detailed_record.h"
#include "configuration.h"
#include "operator.h"
#include "operator_set.h"

namespace call_center {

class ConfigurationAdapter {
 public:
  explicit ConfigurationAdapter(std::shared_ptr<Configuration> configuration);

  void SetOperatorCount(size_t count);
  void SetCallQueueCapacity(size_t capacity);
  void UpdateConfiguration() const;
  void SetConfigurationCaching(bool caching);
  void SetOperatorDelay(Operator::DelayDuration min_delay, Operator::DelayDuration max_delay);
  void SetOperatorDelay(Operator::DelayDuration delay);
  void SetCallMaxWait(CallDetailedRecord::WaitingDuration max_wait);
  void SetMetricsUpdateTime(qs::metrics::QueueingSystemMetrics::MetricsUpdateDuration delay);

 private:
  const std::shared_ptr<Configuration> configuration_;
  boost::json::object config_json;
};

}  // namespace call_center

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_CONFIGURATION_ADAPTER_H_
