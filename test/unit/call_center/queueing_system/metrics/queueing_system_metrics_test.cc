#include "queueing_system/metrics/queueing_system_metrics.h"

#include <gtest/gtest.h>

#include <future>

#include "call_center.h"
#include "configuration_adapter.h"
#include "fake/fake_call_detailed_record.h"
#include "fake/fake_service_loader.h"
#include "fake/fake_task_manager.h"
#include "journal.h"
#include "utils.h"

namespace call_center::qs::metrics::test {
using namespace log;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace core;
using namespace call_center::test;

using CallPtr = std::shared_ptr<CallDetailedRecord>;
using CallsVector = std::vector<CallPtr>;

class QueueingSystemMetricsTest : public testing::Test {
 public:
  QueueingSystemMetricsTest();
  ~QueueingSystemMetricsTest() override;

  [[nodiscard]] CallPtr CreateUniqueCall();

  const std::string test_name_;
  const std::string test_group_name_;
  const LoggerProvider logger_provider_;
  const std::shared_ptr<Configuration> configuration_;
  ConfigurationAdapter configuration_adapter_;
  const std::shared_ptr<FakeTaskManager> task_manager_;
  std::shared_ptr<const ClockAdapter> clock_;
  const std::shared_ptr<QueueingSystemMetrics> metrics_;
  std::shared_ptr<CallCenter> call_center_;
  std::shared_ptr<FakeServiceLoader> service_loader_;
  int next_call_index_;
};

QueueingSystemMetricsTest::QueueingSystemMetricsTest()
    : test_name_(testing::UnitTest::GetInstance()->current_test_info()->name()),
      test_group_name_("QueueingSystemMetricsTest"),
      logger_provider_(std::make_shared<Sink>(
          test_group_name_ + "/logs/" + test_name_ + ".log", SeverityLevel::kTrace, SIZE_MAX
      )),
      configuration_(Configuration::Create(
          logger_provider_, test_group_name_ + "/configs/" + test_name_ + ".json"
      )),
      configuration_adapter_(configuration_),
      task_manager_(FakeTaskManager::Create(logger_provider_)),
      clock_(task_manager_->GetClock()),
      metrics_(
          QueueingSystemMetrics::Create(task_manager_, configuration_, logger_provider_, clock_)
      ),
      call_center_(CallCenter::Create(
          std::make_unique<Journal>(configuration_),
          configuration_,
          task_manager_,
          logger_provider_,
          std::make_unique<OperatorSet>(
              configuration_,
              [this]() {
                return Operator::Create(task_manager_, configuration_, logger_provider_);
              },
              logger_provider_,
              metrics_
          ),
          std::make_unique<CallQueue>(configuration_, logger_provider_),
          metrics_
      )),
      service_loader_(FakeServiceLoader::Create(
          task_manager_,
          call_center_,
          [this] {
            return CreateUniqueCall();
          }
      )),
      next_call_index_(1) {
  CreateDirForLogs(test_group_name_);
  CreateDirForConfigs(test_group_name_);
  configuration_adapter_.SetConfigurationCaching(false);
  configuration_adapter_.UpdateConfiguration();
  task_manager_->Start();
}

QueueingSystemMetricsTest::~QueueingSystemMetricsTest() {
  task_manager_->Stop();
}

CallPtr QueueingSystemMetricsTest::CreateUniqueCall() {
  return FakeCallDetailedRecord::Create(
      clock_, std::to_string(next_call_index_++), configuration_, [](const auto &call) {}
  );
}

template <typename T, typename Avg>
std::ostream &operator<<(std::ostream &out, Metric<T, Avg> metric) {
  out << "min: " << metric.GetMin() << ", ";
  out << "max: " << metric.GetMax() << ", ";
  out << "avg: " << metric.GetAvg() << "";
  return out;
}

double ExpectedUtilization(double lambda, int b) {
  return lambda * b;
}

double ExpectedWaitTime(double lambda, int b) {
  const auto rho = ExpectedUtilization(lambda, b);
  return b * rho / (2 * (1 - rho));
}

double ExpectedQueueSize(double lambda, int b) {
  return ExpectedWaitTime(lambda, b) * lambda;
}

double ExpectedInSystemCount(double lambda, int b) {
  return lambda * (ExpectedWaitTime(lambda, b) + b);
}

TEST_F(QueueingSystemMetricsTest, SystemUtilization) {
  constexpr auto call_max_wait = 1000s;
  constexpr auto operator_count = 1;
  constexpr auto queue_capacity = SIZE_MAX;
  constexpr auto call_count = 1000;
  constexpr auto service_time = 15s;
  constexpr auto arrival_rate = 0.001;
  constexpr auto test_time = seconds(static_cast<uint64_t>(call_count / arrival_rate));
  constexpr auto accuracy = 1e-3;
  constexpr auto metrics_update_time = 15s;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(service_time);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(queue_capacity);
  configuration_adapter_.SetMetricsUpdateTime(metrics_update_time);
  configuration_adapter_.UpdateConfiguration();

  service_loader_->StartLoading(arrival_rate);
  task_manager_->AdvanceTime(test_time);

  const auto expct_utilization = ExpectedUtilization(arrival_rate, service_time.count());
  EXPECT_NEAR(expct_utilization, metrics_->GetBusyServerCountMetric().GetAvg(), accuracy);
}

TEST_F(QueueingSystemMetricsTest, AverageServiceTime) {
  constexpr auto call_max_wait = 1000s;
  constexpr auto operator_count = 1;
  constexpr auto queue_capacity = SIZE_MAX;
  constexpr auto call_count = 100;
  constexpr auto service_time = 15s;
  constexpr auto arrival_rate = 0.001;
  constexpr auto test_time = seconds(static_cast<uint64_t>(call_count / arrival_rate));
  constexpr auto metrics_update_time = 15s;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(service_time);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(queue_capacity);
  configuration_adapter_.SetMetricsUpdateTime(metrics_update_time);
  configuration_adapter_.UpdateConfiguration();

  service_loader_->StartLoading(arrival_rate);
  task_manager_->AdvanceTime(test_time);

  EXPECT_EQ(service_time.count(), floor<seconds>(metrics_->GetAverageServiceTime()).count());
}

TEST_F(QueueingSystemMetricsTest, AverageRequestsCountInSystem) {
  constexpr auto call_max_wait = 1000s;
  constexpr auto operator_count = 1;
  constexpr auto queue_capacity = SIZE_MAX;
  constexpr auto call_count = 1000;
  constexpr auto service_time = 15s;
  constexpr auto arrival_rate = 0.001;
  constexpr auto test_time = seconds(static_cast<uint64_t>(call_count / arrival_rate));
  constexpr auto accuracy = 1e-3;
  constexpr auto metrics_update_time = 15s;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(service_time);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(queue_capacity);
  configuration_adapter_.SetMetricsUpdateTime(metrics_update_time);
  configuration_adapter_.UpdateConfiguration();

  service_loader_->StartLoading(arrival_rate);
  task_manager_->AdvanceTime(test_time);

  const auto expt_in_system_count = ExpectedInSystemCount(arrival_rate, service_time.count());
  EXPECT_NEAR(expt_in_system_count, metrics_->GetRequestCountInSystemMetric().GetAvg(), accuracy);
}

TEST_F(QueueingSystemMetricsTest, AverageQueueSize) {
  constexpr auto call_max_wait = 1000s;
  constexpr auto operator_count = 1;
  constexpr auto queue_capacity = SIZE_MAX;
  constexpr auto call_count = 1000;
  constexpr auto service_time = 15s;
  constexpr auto arrival_rate = 0.001;
  constexpr auto test_time = seconds(static_cast<uint64_t>(call_count / arrival_rate));
  constexpr auto accuracy = 1e-3;
  constexpr auto metrics_update_time = 15s;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(service_time);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(queue_capacity);
  configuration_adapter_.SetMetricsUpdateTime(metrics_update_time);
  configuration_adapter_.UpdateConfiguration();

  service_loader_->StartLoading(arrival_rate);
  task_manager_->AdvanceTime(test_time);

  const auto expct_queue_size = ExpectedQueueSize(arrival_rate, service_time.count());
  EXPECT_NEAR(expct_queue_size, metrics_->GetQueueSizeMetric().GetAvg(), accuracy);
}

}  // namespace call_center::qs::metrics::test
