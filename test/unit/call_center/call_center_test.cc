#include "call_center.h"

#include <gtest/gtest.h>

#include "configuration_adapter.h"
#include "fake/clock_interface.h"
#include "fake/fake_call_detailed_record.h"
#include "fake/fake_task_manager.h"

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

  ~CallCenterTest() override {
    VerifyAllDone();
  }

  void VerifyAllDone() const {
    EXPECT_EQ(0, call_queue_->GetSize()) << "The test time is over, but the work is not finished";
    EXPECT_EQ(0, operators_->GetBusyOperatorCount())
        << "The test time is over, but the work is not finished";
  }

  [[nodiscard]] std::shared_ptr<CallDetailedRecord> CreateCall(const std::string &caller_number
  ) const {
    return FakeCallDetailedRecord::Create(
        clock_, caller_number, configuration_, [](const auto &call) {}
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
  const std::shared_ptr<FakeTaskManager> task_manager_ = FakeTaskManager::Create(logger_provider_);
  const ClockInterface &clock_ = task_manager_->GetClock();
  Journal *journal_;
  OperatorSet *operators_;
  CallQueue *call_queue_;
  std::shared_ptr<CallCenter> call_center_;
};

template <typename Duration>
void VerifyCallResult(
    const CallDetailedRecord &call, CallStatus finish_status, Duration handling_duration
) {
  EXPECT_EQ(finish_status, call.GetStatus())
      << "The call with number '" << call.GetCallerPhoneNumber()
      << "' was processed with a different result";
  EXPECT_EQ(
      handling_duration, std::chrono::floor<Duration>(call.GetTotalTime().value_or(Duration(-1)))
  ) << "The call with number '"
    << call.GetCallerPhoneNumber() << "' was processed with a different time";
}

TEST_F(CallCenterTest, RejectWithTimout) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 1s;

  configuration_adapter_.SetOperatorCount(1);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto first_call = CreateCall("1");
  const auto second_call = CreateCall("2");

  call_center_->PushCall(first_call);
  call_center_->PushCall(second_call);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallResult(*first_call, CallStatus::kOk, operator_delay);
  VerifyCallResult(*second_call, CallStatus::kTimeout, call_max_wait);
}

TEST_F(CallCenterTest, TwoCallsRejectedWithTimeout) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 1s;
  const auto call3_wait_time = 500ms;

  configuration_adapter_.SetOperatorCount(1);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto call1 = CreateCall("1");
  const auto call2 = CreateCall("2");
  const auto call3 = CreateCall("3");

  call_center_->PushCall(call1);
  call_center_->PushCall(call2);
  task_manager_->AdvanceTime(operator_delay - call3_wait_time);
  call_center_->PushCall(call3);
  task_manager_->AdvanceTime(call3_wait_time + operator_delay);
  task_manager_->Stop();

  VerifyCallResult(*call1, CallStatus::kOk, operator_delay);
  VerifyCallResult(*call2, CallStatus::kTimeout, call_max_wait);
  VerifyCallResult(*call3, CallStatus::kOk, call3_wait_time + operator_delay);
}

}  // namespace call_center::test