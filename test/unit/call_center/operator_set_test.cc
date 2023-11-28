#include "operator_set.h"

#include <gtest/gtest.h>

#include <future>

#include "configuration_adapter.h"
#include "core/task_manager_impl.h"
#include "fake/fake_operator.h"
#include "log/logger.h"
#include "log/logger_provider.h"
#include "utils.h"

namespace call_center::test {

using namespace call_center::log;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace call_center::core;
using namespace call_center::qs::metrics;

using OperatorPtr = std::shared_ptr<Operator>;
using Operators = std::vector<OperatorPtr>;

class OperatorSetTest : public testing::Test {
 public:
  OperatorSetTest();

  Operators EraseOperators(size_t count);
  void FreeOperators(const Operators &operators);

  const std::string test_name_;
  const std::string test_group_name_;
  const LoggerProvider logger_provider_;
  const std::shared_ptr<Configuration> configuration_;
  ConfigurationAdapter configuration_adapter_;
  const std::shared_ptr<TaskManagerImpl> task_manager_;
  std::shared_ptr<QueueingSystemMetrics> metrics_;
  OperatorSet operator_set_;
  std::shared_ptr<Logger> logger_;
  std::future<OperatorPtr> EraseOperatorMt();
  Operators EraseOperatorsMt(size_t count);
  std::future<void> FreeOperatorMt(const OperatorPtr &op);
  void FreeOperatorsMt(const Operators &operators);
};

OperatorSetTest::OperatorSetTest()
    : test_name_(testing::UnitTest::GetInstance()->current_test_info()->name()),
      test_group_name_("OperatorSetTest"),
      logger_provider_(std::make_shared<Sink>(
          test_group_name_ + "/logs/" + test_name_ + ".log", SeverityLevel::kTrace, SIZE_MAX
      )),
      configuration_(Configuration::Create(
          logger_provider_, test_group_name_ + "/configs/" + test_name_ + ".json"
      )),
      configuration_adapter_(configuration_),
      task_manager_(TaskManagerImpl::Create(configuration_, logger_provider_)),
      metrics_(QueueingSystemMetrics::Create(task_manager_, configuration_, logger_provider_)),
      operator_set_(
          configuration_,
          [logger_provider = logger_provider_, config = configuration_]() {
            return FakeOperator::Create(config, logger_provider);
          },
          logger_provider_,
          metrics_
      ),
      logger_(logger_provider_.Get(test_group_name_)) {
  CreateDirForLogs(test_group_name_);
  CreateDirForConfigs(test_group_name_);
  configuration_adapter_.SetConfigurationCaching(false);
  configuration_adapter_.SetOperatorCount(10);
  configuration_adapter_.UpdateConfiguration();
  task_manager_->Start();
}

Operators OperatorSetTest::EraseOperators(size_t count) {
  std::vector<OperatorPtr> free_operators(count);
  for (size_t i = 0; i < count; ++i) {
    free_operators[i] = operator_set_.EraseFree();
  }
  return free_operators;
}

void OperatorSetTest::FreeOperators(const Operators &operators) {
  for (const auto &op : operators) {
    operator_set_.InsertFree(op);
  }
}

std::future<OperatorPtr> OperatorSetTest::EraseOperatorMt() {
  std::promise<OperatorPtr> promise;
  auto future = promise.get_future();
  task_manager_->PostTaskDelayed(
      1s,
      [&operator_set = operator_set_, promise = CapturePromise(std::move(promise))]() {
        promise->set_value(operator_set.EraseFree());
      }
  );
  return future;
}

std::future<void> OperatorSetTest::FreeOperatorMt(const OperatorPtr &op) {
  std::promise<void> promise;
  auto future = promise.get_future();
  task_manager_->PostTaskDelayed(
      1s,
      [op = op, &operator_set = operator_set_, promise = CapturePromise(std::move(promise))]() {
        operator_set.InsertFree(op);
        promise->set_value();
      }
  );
  return future;
}

void OperatorSetTest::FreeOperatorsMt(const Operators &operators) {
  std::vector<std::future<void>> futures(operators.size());
  for (size_t i = 0; i < operators.size(); ++i) {
    futures[i] = FreeOperatorMt(operators[i]);
  }
  for (size_t i = 0; i < operators.size(); ++i) {
    futures[i].get();
  }
}

Operators OperatorSetTest::EraseOperatorsMt(size_t count) {
  std::vector<std::future<OperatorPtr>> futures(count);
  for (size_t i = 0; i < count; ++i) {
    futures[i] = EraseOperatorMt();
  }
  Operators ops(count);
  for (size_t i = 0; i < count; ++i) {
    ops[i] = futures[i].get();
  }
  return ops;
}

void EraseNullOperators(Operators &operators) {
  std::erase_if(operators, [](const auto &op) {
    return op == nullptr;
  });
}

TEST_F(OperatorSetTest, SetOperatorCountInConfig) {
  const auto new_operator_count = operator_set_.GetSize() * 2;

  configuration_adapter_.SetOperatorCount(new_operator_count);
  configuration_adapter_.UpdateConfiguration();
  const auto op = operator_set_.EraseFree();

  EXPECT_EQ(new_operator_count, operator_set_.GetSize());
  EXPECT_EQ(new_operator_count - 1, operator_set_.GetFreeOperatorCount());
  EXPECT_EQ(1, operator_set_.GetBusyOperatorCount());
}

TEST_F(OperatorSetTest, TryEraseWithFreeCount_GetNullptr) {
  const auto operator_count = operator_set_.GetSize();
  EraseOperators(operator_count);

  const auto op = operator_set_.EraseFree();

  EXPECT_EQ(nullptr, op);
}

TEST_F(OperatorSetTest, EraseDifferentOperators) {
  const auto operator_count = operator_set_.GetSize();

  std::vector<OperatorPtr> free_operators = EraseOperators(operator_count);
  std::vector<OperatorPtr> unique_operators(operator_count);
  std::unique_copy(free_operators.begin(), free_operators.end(), unique_operators.begin());
  EXPECT_EQ(unique_operators, free_operators);
}

TEST_F(OperatorSetTest, FreeOperatorAfterErase) {
  const auto operator_count = operator_set_.GetSize();
  const auto to_erase_count = operator_count / 2;

  std::vector<OperatorPtr> free_operators = EraseOperators(to_erase_count);
  FreeOperators(free_operators);

  EXPECT_EQ(0, operator_set_.GetBusyOperatorCount());
  EXPECT_EQ(operator_count, operator_set_.GetFreeOperatorCount());
  EXPECT_EQ(operator_count, operator_set_.GetSize());
}

TEST_F(OperatorSetTest, EraseOperatorsInDifferentThreads) {
  const auto operator_count = operator_set_.GetSize();
  const auto to_erase_count = operator_count * 2;

  std::vector<OperatorPtr> free_operators = EraseOperatorsMt(to_erase_count);
  EraseNullOperators(free_operators);

  EXPECT_EQ(operator_count, free_operators.size());
  EXPECT_EQ(0, operator_set_.GetFreeOperatorCount());
  EXPECT_EQ(operator_count, operator_set_.GetBusyOperatorCount());
}

TEST_F(OperatorSetTest, EraseAndFreeOperatorsInDifferentThreads) {
  const auto operator_count = operator_set_.GetSize();
  const auto to_erase_count = operator_count * 2;

  std::vector<OperatorPtr> free_operators = EraseOperatorsMt(to_erase_count);
  EraseNullOperators(free_operators);
  FreeOperatorsMt(free_operators);

  EXPECT_EQ(operator_count, operator_set_.GetSize());
  EXPECT_EQ(operator_count, operator_set_.GetFreeOperatorCount());
  EXPECT_EQ(0, operator_set_.GetBusyOperatorCount());
}

TEST_F(OperatorSetTest, DecreseOperatorCountViaConfig) {
  const auto operator_count = operator_set_.GetSize();
  const auto new_operator_count = operator_count / 2;

  configuration_adapter_.SetOperatorCount(new_operator_count);
  configuration_adapter_.UpdateConfiguration();
  std::vector<OperatorPtr> free_operators = EraseOperators(operator_count);
  EraseNullOperators(free_operators);

  EXPECT_EQ(new_operator_count, free_operators.size());
  EXPECT_EQ(0, operator_set_.GetFreeOperatorCount());
  EXPECT_EQ(new_operator_count, operator_set_.GetBusyOperatorCount());
}

}  // namespace call_center::test
