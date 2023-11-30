#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_TASK_MANAGER_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_TASK_MANAGER_H_

#include <boost/asio.hpp>
#include <map>

#include "core/clock_adapter.h"
#include "core/task_manager.h"
#include "core/tasks.h"
#include "fake_clock.h"

namespace call_center::core {

class FakeTaskManager : public TaskManager {
 public:
  static std::shared_ptr<FakeTaskManager> Create(const log::LoggerProvider &logger_provider);

  void Start() override;
  void Stop() override;
  void Join() override;
  boost::asio::io_context &IoContext() override;
  void PostTask(std::function<Task> task) override;
  void PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) override;
  void PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) override;
  void AdvanceTime(Duration_t duration);
  std::shared_ptr<const ClockAdapter> GetClock() const;

 private:
  static const size_t kThreadCount;

  std::multimap<FakeClock::TimePoint, tasks::TaskWrapped<Task>> tasks_;
  std::shared_ptr<FakeClock> clock_;
  boost::thread_group threads_;
  const std::unique_ptr<log::Logger> logger_;
  bool started_ = false;
  bool stopped_ = false;
  mutable std::shared_mutex start_mutex_;
  mutable std::shared_mutex tasks_mutex_;
  mutable std::mutex advance_mutex_;
  std::atomic_int handle_count_ = 0;
  std::condition_variable_any has_tasks_;
  std::condition_variable_any done_tasks_;

  explicit FakeTaskManager(const log::LoggerProvider &logger_provider);

  tasks::TaskWrapped<Task> MakeTaskWrapped(std::function<Task> task);
  void AddTask(FakeClock::TimePoint time_point, std::function<Task> task);
  bool HasTasks() const;
  void HandleTasks();
  void HandleFirstTask();
  [[nodiscard]] bool IsStopped() const;
  void ClearTasks();
};

}  // namespace call_center::core

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_TASK_MANAGER_H_
