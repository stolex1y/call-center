#include "core/task_manager_impl.h"

#include <gtest/gtest.h>

#include <future>
#include <iostream>

#include "configuration_adapter.h"
#include "utils.h"

namespace call_center::core::test {

using namespace log;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace call_center::test;

using TaskResult = std::future<bool>;
using TaskResults = std::vector<TaskResult>;

class TaskManagerImplTest : public testing::Test {
 public:
  TaskManagerImplTest();
  ~TaskManagerImplTest() override;

  TaskResult PostTask(size_t id);
  TaskResults PostTasks(size_t task_count);
  template<typename TimePoint>
  TaskResult PostTaskAt(size_t id, const TimePoint &time_point);
  template<typename TimePoint>
  TaskResults PostTasksAt(const TimePoint &time_point, size_t task_count);
  template<typename Duration>
  TaskResult PostTaskDelayed(size_t id, const Duration &delay);
  template<typename Duration>
  TaskResults PostTasksDelayed(const Duration &delay, size_t task_count);

  const std::string test_name_;
  const std::string test_group_name_;
  const std::shared_ptr<const LoggerProvider> logger_provider_;
  const std::shared_ptr<Configuration> configuration_;
  ConfigurationAdapter configuration_adapter_;
  const std::shared_ptr<TaskManagerImpl> task_manager_;
  std::shared_ptr<Logger> logger_;
};

TaskManagerImplTest::TaskManagerImplTest()
    : test_name_(testing::UnitTest::GetInstance()->current_test_info()->name()),
      test_group_name_("TaskManagerImplTest"),
      logger_provider_(std::make_shared<LoggerProvider>(std::make_unique<Sink>(
          test_group_name_ + "/logs/" + test_name_ + ".log", SeverityLevel::kTrace, SIZE_MAX
      ))),
      configuration_(Configuration::Create(
          logger_provider_, test_group_name_ + "/configs/" + test_name_ + ".json"
      )),
      configuration_adapter_(configuration_),
      task_manager_(TaskManagerImpl::Create(configuration_, logger_provider_)),
      logger_(logger_provider_->Get(test_group_name_)) {
  CreateDirForLogs(test_group_name_);
  CreateDirForConfigs(test_group_name_);
  configuration_adapter_.SetConfigurationCaching(false);
  configuration_adapter_.UpdateConfiguration();
  task_manager_->Start();
}

TaskManagerImplTest::~TaskManagerImplTest() {
  task_manager_->Stop();
}

template<typename T>
auto CapturePromise(std::promise<T> &&promise) {
  return std::make_shared<std::promise<T>>(std::move(promise));
}

TaskResult TaskManagerImplTest::PostTask(size_t id) {
  std::promise<bool> promise;
  TaskResult future = promise.get_future();
  task_manager_->PostTask([id, logger = logger_, promise = CapturePromise(std::move(promise))](
  ) mutable {
    promise->set_value(true);
    logger->Info() << "Task '" << id << "' executed";
  });
  return future;
}

TaskResults TaskManagerImplTest::PostTasks(size_t task_count) {
  TaskResults results(task_count);
  for (size_t i = 0; i < task_count; ++i) {
    results[i] = PostTask(i + 1);
  }
  return results;
}

template<typename Duration>
TaskResults TaskManagerImplTest::PostTasksDelayed(const Duration &delay, size_t task_count) {
  TaskResults results(task_count);
  for (size_t i = 0; i < task_count; ++i) {
    results[i] = PostTaskDelayed(i + 1, delay);
  }
  return results;
}

template<typename Duration>
TaskResult TaskManagerImplTest::PostTaskDelayed(size_t id, const Duration &delay) {
  std::promise<bool> promise;
  TaskResult future = promise.get_future();
  task_manager_->PostTaskDelayed(
      delay,
      [id, logger = logger_, promise = CapturePromise(std::move(promise))]() mutable {
        promise->set_value(true);
        logger->Info() << "Delayed task '" << id << "' executed";
      }
  );
  return future;
}

template<typename TimePoint>
TaskResults TaskManagerImplTest::PostTasksAt(const TimePoint &time_point, size_t task_count) {
  TaskResults results(task_count);
  for (size_t i = 0; i < task_count; ++i) {
    results[i] = PostTaskAt(i + 1, time_point);
  }
  return results;
}

template<typename TimePoint>
TaskResult TaskManagerImplTest::PostTaskAt(size_t id, const TimePoint &time_point) {
  std::promise<bool> promise;
  TaskResult future = promise.get_future();
  task_manager_->PostTaskAt(
      time_point,
      [id, logger = logger_, promise = CapturePromise(std::move(promise))]() mutable {
        promise->set_value(true);
        logger->Info() << "Deferred task '" << id << "' executed";
      }
  );
  return future;
}

template<typename Duration>
void VerifyTaskResult(TaskResult &result, const Duration &delay) {
  std::future_status status = result.wait_for(delay);
  ASSERT_EQ(std::future_status::ready, status) << "Task result must be ready.";
  ASSERT_TRUE(result.get()) << "Task result must be true.";
}

template<typename Duration = std::chrono::milliseconds>
void VerifyTaskResults(TaskResults &results, const Duration &delay = 500ms) {
  const auto start = steady_clock::now();
  for (auto &result : results) {
    ASSERT_GE(delay, steady_clock::now() - start)
                  << "Exceeded the time limit for waiting for the result of the work.";
    VerifyTaskResult(result, delay);
  }
}

template<typename TimePoint>
void VerifyTaskResultsAt(const TimePoint &time_point, TaskResults &results) {
  while (utc_clock::now() < time_point) {
    for (auto &result : results) {
      const auto status = result.wait_for(0ns);
      if (utc_clock::now() < time_point) {
        ASSERT_EQ(std::future_status::timeout, status) << "Task result mustn't be ready.";
      }
    }
    std::this_thread::sleep_for(1ms);
  }
  VerifyTaskResults(results);
}

template<typename Duration>
void VerifyTaskResultsDelayed(const Duration &delay, TaskResults &results) {
  const auto waiting_time_end = utc_clock::now() + delay;
  while (utc_clock::now() < waiting_time_end) {
    for (auto &result : results) {
      const auto status = result.wait_for(0ns);
      if (utc_clock::now() < waiting_time_end) {
        ASSERT_EQ(std::future_status::timeout, status) << "Task result mustn't be ready.";
      }
    }
    std::this_thread::sleep_for(1ms);
  }
  VerifyTaskResults(results);
}

TEST_F(TaskManagerImplTest, PostTask_TaskExecuted) {
  auto results = PostTasks(std::max(task_manager_->GetUserThreadCount() / 3, size_t(1)));
  VerifyTaskResults(results);
}

TEST_F(TaskManagerImplTest, PostTaskAt_TaskExecuted) {
  const auto future_time_point = utc_clock::now() + 2s;
  auto results =
      PostTasksAt(future_time_point, std::max(task_manager_->GetUserThreadCount() / 3, size_t(1)));
  VerifyTaskResultsAt(future_time_point, results);
}

TEST_F(TaskManagerImplTest, PostTaskDelayed_TaskExecuted) {
  const auto delay = 2s;
  auto results =
      PostTasksDelayed(delay, std::max(task_manager_->GetUserThreadCount() / 3, size_t(1)));
  VerifyTaskResultsDelayed(delay, results);
}

}  // namespace call_center::core::test
