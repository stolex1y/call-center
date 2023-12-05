#include "call_center.h"

#include <gtest/gtest.h>

#include "configuration_adapter.h"
#include "fake/fake_call_detailed_record.h"
#include "fake/fake_task_manager.h"
#include "utils.h"

namespace call_center::test {

using namespace log;
using namespace core;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace qs::metrics;
using namespace core::tasks::test;

using CallPtr = std::shared_ptr<CallDetailedRecord>;
using CallsVector = std::vector<CallPtr>;

class CallCenterTest : public testing::Test {
 public:
  CallCenterTest();
  ~CallCenterTest() override;

  void VerifyAllDone() const;
  [[nodiscard]] CallPtr DuplicateCall(const CallPtr &call) const;
  [[nodiscard]] CallsVector DuplicateCall(const CallPtr &call, size_t count) const;
  [[nodiscard]] CallsVector DuplicateCalls(const CallsVector &calls) const;
  [[nodiscard]] CallPtr CreateUniqueCall();
  [[nodiscard]] CallsVector CreateUniqueCalls(size_t count);
  void PushCalls(const CallsVector &calls) const;
  void PushCall(const CallPtr &call) const;

  const std::string test_name_;
  const std::string test_group_name_;
  const LoggerProvider logger_provider_;
  const std::shared_ptr<Configuration> configuration_;
  ConfigurationAdapter configuration_adapter_;
  const std::shared_ptr<FakeTaskManager> task_manager_;
  std::shared_ptr<const ClockAdapter> clock_;
  Journal *journal_;
  const std::shared_ptr<QueueingSystemMetrics> metrics_;
  OperatorSet *operators_;
  CallQueue *call_queue_;
  std::shared_ptr<CallCenter> call_center_;
  int next_call_index_;
};

CallCenterTest::CallCenterTest()
    : test_name_(testing::UnitTest::GetInstance()->current_test_info()->name()),
      test_group_name_("CallCenterTest"),
      logger_provider_(std::make_shared<Sink>(
          test_group_name_ + "/logs/" + test_name_ + ".log", SeverityLevel::kTrace, SIZE_MAX
      )),
      configuration_(Configuration::Create(
          logger_provider_, test_group_name_ + "/configs/" + test_name_ + ".json"
      )),
      configuration_adapter_(configuration_),
      task_manager_(FakeTaskManager::Create(logger_provider_)),
      clock_(task_manager_->GetClock()),
      journal_(new Journal(configuration_)),
      metrics_(QueueingSystemMetrics::Create(task_manager_, configuration_, logger_provider_)),
      operators_(new OperatorSet(
          configuration_,
          [this]() {
            return Operator::Create(task_manager_, configuration_, logger_provider_);
          },
          logger_provider_,
          metrics_
      )),
      call_queue_(new CallQueue(configuration_, logger_provider_)),
      call_center_(CallCenter::Create(
          std::unique_ptr<Journal>(journal_),
          configuration_,
          task_manager_,
          logger_provider_,
          std::unique_ptr<OperatorSet>(operators_),
          std::unique_ptr<CallQueue>(call_queue_),
          metrics_
      )),
      next_call_index_(1) {
  CreateDirForLogs(test_group_name_);
  CreateDirForConfigs(test_group_name_);
  configuration_adapter_.SetConfigurationCaching(false);
  configuration_adapter_.UpdateConfiguration();
  task_manager_->Start();
}

CallCenterTest::~CallCenterTest() {
  VerifyAllDone();
}

void CallCenterTest::PushCall(const CallPtr &call) const {
  call_center_->PushCall(call);
}

void CallCenterTest::PushCalls(const CallsVector &calls) const {
  for (const auto &call : calls) {
    PushCall(call);
  }
}

CallPtr CallCenterTest::CreateUniqueCall() {
  return FakeCallDetailedRecord::Create(
      clock_, std::to_string(next_call_index_++), configuration_, [](const auto &call) {}
  );
}

CallsVector CallCenterTest::CreateUniqueCalls(const size_t count) {
  CallsVector calls(count);
  for (size_t i = 0; i < count; ++i) {
    calls[i] = CreateUniqueCall();
  }
  return calls;
}

CallPtr CallCenterTest::DuplicateCall(const CallPtr &call) const {
  return FakeCallDetailedRecord::Create(
      clock_, call->GetCallerPhoneNumber(), configuration_, [](const auto &c) {}
  );
}

CallsVector CallCenterTest::DuplicateCall(const CallPtr &call, const size_t count) const {
  CallsVector clones(count);
  for (size_t i = 0; i < count; ++i) {
    clones[i] = DuplicateCall(call);
  }
  return clones;
}

CallsVector CallCenterTest::DuplicateCalls(const CallsVector &calls) const {
  CallsVector clones(calls.size());
  for (size_t i = 0; i < calls.size(); ++i) {
    clones[i] = DuplicateCall(calls[i]);
  }
  return clones;
}

void CallCenterTest::VerifyAllDone() const {
  EXPECT_EQ(0, call_queue_->GetSize()) << "The test time is over, but the work is not finished";
  EXPECT_EQ(0, operators_->GetBusyOperatorCount())
      << "The test time is over, but the work is not finished";
}

template <typename Duration>
void VerifyCallResult(
    const CallDetailedRecord &call, const CallStatus finish_status, Duration handling_duration
) {
  EXPECT_EQ(finish_status, call.GetStatus())
      << "The call with number '" << call.GetCallerPhoneNumber()
      << "' was processed with a different result";
  EXPECT_EQ(
      handling_duration, std::chrono::floor<Duration>(call.GetTotalTime().value_or(Duration(-1)))
  ) << "The call with number '"
    << call.GetCallerPhoneNumber() << "' was processed with a different time";
}

template <typename Duration>
void VerifyCallsResult(
    const CallsVector &calls, CallStatus finish_status, Duration handling_duration
) {
  for (const auto &call : calls) {
    VerifyCallResult(*call, finish_status, handling_duration);
  }
}

TEST_F(CallCenterTest, AllOperatorsAreBusy_CallRejectedWithTimout) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 1s;
  constexpr auto operator_count = 5;
  constexpr auto call_in_queue_count = 10;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto processed_calls = CreateUniqueCalls(operator_count);
  const auto timeout_calls = CreateUniqueCalls(call_in_queue_count);

  PushCalls(processed_calls);
  PushCalls(timeout_calls);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(processed_calls, CallStatus::kOk, operator_delay);
  VerifyCallsResult(timeout_calls, CallStatus::kTimeout, call_max_wait);
}

TEST_F(CallCenterTest, OperatorsAreReleasedAfterProcessingCalls) {
  const auto operator_delay = 4s;
  const auto call_max_wait = 2s;
  constexpr auto operator_count = 5;
  constexpr auto timeout_calls_count = 5;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto processed_calls = CreateUniqueCalls(operator_count);
  const auto timeout_calls = CreateUniqueCalls(timeout_calls_count);
  const auto second_processed_calls = CreateUniqueCalls(operator_count);

  PushCalls(processed_calls);
  PushCalls(timeout_calls);
  task_manager_->AdvanceTime(operator_delay - call_max_wait / 2);
  PushCalls(second_processed_calls);
  task_manager_->AdvanceTime(call_max_wait / 2 + operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(processed_calls, CallStatus::kOk, operator_delay);
  VerifyCallsResult(timeout_calls, CallStatus::kTimeout, call_max_wait);
  VerifyCallsResult(second_processed_calls, CallStatus::kOk, call_max_wait / 2 + operator_delay);
}

TEST_F(CallCenterTest, FullQueue_CallRejectedWithOverload) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 1s;
  constexpr auto operator_count = 5;
  constexpr auto queue_capacity = 5;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(queue_capacity);
  configuration_adapter_.UpdateConfiguration();

  const auto calls_to_busy_operators = CreateUniqueCalls(operator_count);
  const auto calls_to_full_queue = CreateUniqueCalls(queue_capacity);
  const auto overloaded_calls = CreateUniqueCalls(queue_capacity);

  PushCalls(calls_to_busy_operators);
  PushCalls(calls_to_full_queue);
  PushCalls(overloaded_calls);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(calls_to_busy_operators, CallStatus::kOk, operator_delay);
  VerifyCallsResult(calls_to_full_queue, CallStatus::kTimeout, call_max_wait);
  VerifyCallsResult(overloaded_calls, CallStatus::kOverload, 0s);
}

TEST_F(CallCenterTest, HasFreeOperators_CallsProcessed) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 10s;
  constexpr auto operator_count = 5;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto processed_calls = CreateUniqueCalls(operator_count);

  PushCalls(processed_calls);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(processed_calls, CallStatus::kOk, operator_delay);
}

TEST_F(CallCenterTest, FirstCallInProcessing_SecondIsClone_FirstIsOk_SecondIsRejected) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 10s;
  constexpr auto operator_count = 5;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto processed_call = CreateUniqueCall();
  const auto first_call_clones = DuplicateCall(processed_call, operator_count - 1);

  PushCall(processed_call);
  PushCalls(first_call_clones);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallResult(*processed_call, CallStatus::kOk, operator_delay);
  VerifyCallsResult(first_call_clones, CallStatus::kAlreadyInQueue, 0s);
}

TEST_F(CallCenterTest, FirstGroupInProcessing_SecondGroupIsClone_FirstGroupIsOk_SecondIsRejected) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 10s;
  constexpr auto operator_count = 10;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto processed_calls = CreateUniqueCalls(operator_count / 2);
  const auto processed_call_clones = DuplicateCalls(processed_calls);

  PushCalls(processed_calls);
  PushCalls(processed_call_clones);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(processed_calls, CallStatus::kOk, operator_delay);
  VerifyCallsResult(processed_call_clones, CallStatus::kAlreadyInQueue, 0s);
}

TEST_F(CallCenterTest, FirstGroupInQueue_SecondGroupIsClone_FirstGroupIsOk_SecondIsRejected) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 10s;
  constexpr auto operator_count = 10;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto calls_to_busy_operators = CreateUniqueCalls(operator_count);
  const auto queued_calls = CreateUniqueCalls(operator_count);
  const auto queued_calls_clones = DuplicateCalls(queued_calls);

  PushCalls(calls_to_busy_operators);
  PushCalls(queued_calls);
  PushCalls(queued_calls_clones);
  task_manager_->AdvanceTime(operator_delay * 2);
  task_manager_->Stop();

  VerifyCallsResult(calls_to_busy_operators, CallStatus::kOk, operator_delay);
  VerifyCallsResult(queued_calls, CallStatus::kOk, operator_delay * 2);
  VerifyCallsResult(queued_calls_clones, CallStatus::kAlreadyInQueue, 0s);
}

TEST_F(CallCenterTest, ZeroQueueCapacity_HasFreeOperators_CallsAreOk) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 10s;
  constexpr auto operator_count = 10;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(0);
  configuration_adapter_.UpdateConfiguration();

  const auto calls = CreateUniqueCalls(operator_count);

  PushCalls(calls);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(calls, CallStatus::kOk, operator_delay);
}

TEST_F(CallCenterTest, ZeroCallMaxWait_HasFreeOperators_CallsAreOk) {
  const auto operator_delay = 3s;
  const auto call_max_wait = 0s;
  constexpr auto operator_count = 10;

  configuration_adapter_.SetOperatorCount(operator_count);
  configuration_adapter_.SetOperatorDelay(operator_delay);
  configuration_adapter_.SetCallMaxWait(call_max_wait);
  configuration_adapter_.SetCallQueueCapacity(SIZE_MAX);
  configuration_adapter_.UpdateConfiguration();

  const auto calls = CreateUniqueCalls(operator_count);

  PushCalls(calls);
  task_manager_->AdvanceTime(operator_delay);
  task_manager_->Stop();

  VerifyCallsResult(calls, CallStatus::kOk, operator_delay);
}

}  // namespace call_center::test
