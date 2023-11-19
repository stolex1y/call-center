#ifndef CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_
#define CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_

#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <chrono>
#include <functional>
#include <thread>

#include "configuration.h"
#include "log/logger.h"
#include "log/sink.h"
#include "tasks.h"
#include "utils/date_time.h"

namespace call_center::core {

class TaskManager {
 public:
  explicit TaskManager(
      std::shared_ptr<const Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider);
  TaskManager(const TaskManager &other) = delete;
  TaskManager &operator=(const TaskManager &other) = delete;

  void Start();
  void Stop();
  void Join();
  boost::asio::io_context &IoContext();

  template <typename Task>
  void PostTask(std::function<Task> task);

  template <typename Task, typename Time>
  void PostTaskAt(const Time &time_point, std::function<Task> task);

  template <typename Task, typename Duration>
  void PostTaskDelayed(const Duration &delay, std::function<Task> task);

 private:
  template <typename Task>
  tasks::TaskWrapped<Task> MakeTaskWrapped(std::function<Task> task);

  template <typename Task, typename Clock>
  tasks::TimerTaskWrapped<Task, Clock> MakeTimerTaskWrapped(
      std::function<Task> task,
      std::unique_ptr<typename tasks::TimerTaskWrapped<Task, Clock>::Timer>
          timer);

  boost::asio::io_context ioc_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;
  std::thread thread_;
  bool stopped_ = true;
  std::mutex mutex_;
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<const Configuration> configuration_;
};

template <typename Task, typename TimePoint>
void TaskManager::PostTaskAt(const TimePoint &time_point,
                             std::function<Task> task) {
  using Clock = typename TimePoint::clock;

  logger_->Info() << "Starting the timer of deferred task until " << time_point;
  auto timer =
      std::make_unique<typename tasks::TimerTaskWrapped<Task, Clock>::Timer>(
          ioc_, time_point);
  timer->async_wait(
      MakeTimerTaskWrapped<Task, Clock>(std::move(task), std::move(timer)));
}

template <typename Task, typename Duration>
void TaskManager::PostTaskDelayed(const Duration &delay,
                                  std::function<Task> task) {
  using Clock = typename std::chrono::steady_clock;

  logger_->Info() << "Starting the timer of task delayed by "
                  << std::chrono::floor<std::chrono::milliseconds>(delay);

  auto timer =
      std::make_unique<typename tasks::TimerTaskWrapped<Task, Clock>::Timer>(
          ioc_, delay);
  timer->async_wait(
      MakeTimerTaskWrapped<Task, Clock>(std::move(task), std::move(timer)));
}

template <typename Task, typename Clock>
tasks::TimerTaskWrapped<Task, Clock> TaskManager::MakeTimerTaskWrapped(
    std::function<Task> task,
    std::unique_ptr<typename tasks::TimerTaskWrapped<Task, Clock>::Timer>
        timer) {
  return {std::move(task), std::move(timer), *logger_};
}

template <typename Task>
tasks::TaskWrapped<Task> TaskManager::MakeTaskWrapped(
    std::function<Task> task) {
  return {std::move(task), *logger_};
}

template <typename Task>
void TaskManager::PostTask(std::function<Task> task) {
  ioc_.post(MakeTaskWrapped(std::move(task)));
}

}  // namespace call_center::core

#endif  // CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_
