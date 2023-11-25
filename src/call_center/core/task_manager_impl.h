#ifndef CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_IMPL_H_
#define CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_IMPL_H_

#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <functional>
#include <thread>

#include "configuration.h"
#include "log/logger.h"
#include "log/sink.h"
#include "task_manager.h"
#include "tasks.h"
#include "utils/date_time.h"

namespace call_center::core {

class TaskManagerImpl : public TaskManager {
 public:
  static constexpr const auto kUserThreadCountKey = "task_manager_user_thread_count";
  static constexpr const auto kIoThreadCountKey = "task_manager_io_thread_count";

  static std::shared_ptr<TaskManagerImpl> Create(
      std::shared_ptr<Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );
  TaskManagerImpl(const TaskManagerImpl &other) = delete;
  TaskManagerImpl &operator=(const TaskManagerImpl &other) = delete;

  void Start() override;
  void Stop() override;
  void Join() override;
  boost::asio::io_context &IoContext() override;
  void PostTask(std::function<Task> task) override;
  [[nodiscard]] size_t GetUserThreadCount() const;
  [[nodiscard]] size_t GetIoThreadCount() const;

 protected:
  void PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) override;
  void PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) override;

 private:
  static const size_t kDefaultUserThreadCount;
  static const size_t kDefaultIoThreadCount;
  boost::asio::io_context io_context_;
  boost::asio::io_context user_context_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> user_work_guard_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> io_work_guard_;
  std::atomic_flag stopped_ = false;
  std::atomic_flag started_ = false;
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<Configuration> configuration_;
  boost::thread_group user_threads_;
  boost::thread_group io_threads_;
  std::atomic_size_t user_thread_count_ = kDefaultUserThreadCount;
  std::atomic_size_t io_thread_count_ = kDefaultIoThreadCount;

  static void AddThreadsToGroup(
      boost::thread_group &thread_group, size_t thread_count, boost::asio::io_context &context
  );

  TaskManagerImpl(
      std::shared_ptr<Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );

  tasks::TaskWrapped<Task> MakeTaskWrapped(std::function<Task> task);
  tasks::TimerTaskWrapped<Task, Clock_t> MakeTimerTaskWrapped(
      std::function<Task> task,
      std::unique_ptr<typename tasks::TimerTaskWrapped<Task, Clock_t>::Timer> timer
  );
  [[nodiscard]] size_t ReadUserThreadCount() const;
  [[nodiscard]] size_t ReadIoThreadCount() const;
};

}  // namespace call_center::core

#endif  // CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_IMPL_H_
