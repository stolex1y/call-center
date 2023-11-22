#include "call_center.h"

#include <gtest/gtest.h>

#include "configuration_adapter.h"
#include "core/task_manager_impl.h"

namespace call_center::test {

using namespace log;
using namespace core;
using namespace std::chrono_literals;

class CallCenterTest : public testing::Test {
 public:
  CallCenterTest()
      : journal_(new Journal(configuration_)),
        operators_(new OperatorSet(configuration_, task_manager_, logger_provider_)),
        call_queue_(new CallQueue(configuration_, logger_provider_)),
        call_center_(CallCenter::Create(
            std::unique_ptr<Journal>(journal_),
            configuration_,
            task_manager_,
            logger_provider_,
            std::unique_ptr<OperatorSet>(operators_),
            std::unique_ptr<CallQueue>(call_queue_)
        )) {
    configuration_adapter_.SetConfigurationCaching(false);
    configuration_adapter_.UpdateConfiguration();
    task_manager_->Start();
  }

  template <typename Duration>
  void LimitTaskManagerExecution(Duration limit) {
    task_manager_->PostTaskDelayed(limit, [this]() {
      EXPECT_EQ(0, call_queue_->GetSize()) << "The test time is over, but the work is not finished";
      EXPECT_EQ(0, operators_->GetBusyOperatorCount())
          << "The test time is over, but the work is not finished";
      task_manager_->Stop();
    });
  }

  template <typename Duration>
  [[nodiscard]] static CallDetailedRecord::OnFinish AssertCallHandlingResult(
      CallStatus finish_status, Duration handling_duration
  ) {
    return [finish_status, handling_duration](const CallDetailedRecord &call) {
      EXPECT_EQ(finish_status, call.GetStatus())
          << "The call with number '" << call.GetCallerPhoneNumber()
          << "' was processed with a different result";
      ASSERT_EQ(
          handling_duration,
          std::chrono::floor<Duration>(call.GetTotalTime().value_or(Duration(-1)))
      ) << "The call with number '"
        << call.GetCallerPhoneNumber() << "' was processed with a different time";
    };
  }

  template <typename Duration>
  [[nodiscard]] std::shared_ptr<CallDetailedRecord> CreateCall(
      const std::string &caller_number, CallStatus finish_status, Duration handling_duration
  ) const {
    return std::make_shared<CallDetailedRecord>(
        caller_number, configuration_, AssertCallHandlingResult(finish_status, handling_duration)
    );
  }

  const std::shared_ptr<const LoggerProvider> logger_provider_ =
      std::make_shared<LoggerProvider>(std::make_unique<Sink>(
          testing::UnitTest::GetInstance()->current_test_info()->name(),
          SeverityLevel::kTrace,
          SIZE_MAX
      ));
  const std::shared_ptr<Configuration> configuration_ = Configuration::Create(logger_provider_);
  ConfigurationAdapter configuration_adapter_{configuration_};
  const std::shared_ptr<TaskManagerImpl> task_manager_ = TaskManagerImpl::Create(configuration_, logger_provider_);
  Journal *journal_;
  OperatorSet *operators_;
  CallQueue *call_queue_;
  std::shared_ptr<CallCenter> call_center_;
};

TEST_F(CallCenterTest, RejectWithTimout) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 1s;

  configuration_adapter_.SetOperatorCount(1);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  call_center_->PushCall(CreateCall("1", CallStatus::kOk, operator_delay));
  call_center_->PushCall(CreateCall("2", CallStatus::kTimeout, call_max_wait));
  LimitTaskManagerExecution(operator_delay);
  task_manager_->Join();
}

}  // namespace call_center::test