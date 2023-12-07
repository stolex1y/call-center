#ifndef CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_IMPL_H_
#define CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_IMPL_H_

#include <boost/asio.hpp>
#include <chrono>
#include <functional>

#include "configuration/configuration.h"
#include "log/logger.h"
#include "log/logger_provider.h"
#include "log/sink.h"
#include "task_manager.h"
#include "tasks.h"

namespace call_center::core::tasks {

/**
 * @brief Реализация @link TaskManager @endlink.
 *
 * Использует два отдельных пула потоков: для задач ввода-вывода и для пользовательских задач.
 */
class TaskManagerImpl : public TaskManager {
 public:
  /// Ключ в конфигурации, соответствующий количеству потоков для пользовательских задач.
  static constexpr auto kUserThreadCountKey = "task_manager_user_thread_count";
  /// Ключ в конфигурации, соответствующий количеству потоков для задач ввода-вывода.
  static constexpr auto kIoThreadCountKey = "task_manager_io_thread_count";

  static std::shared_ptr<TaskManagerImpl> Create(
      std::shared_ptr<config::Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );
  TaskManagerImpl(const TaskManagerImpl &other) = delete;
  TaskManagerImpl &operator=(const TaskManagerImpl &other) = delete;
  ~TaskManagerImpl() override;

  void Start() final;
  void Stop() final;
  void Join() final;
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
  bool stopped_ = false;
  bool started_ = false;
  mutable std::mutex start_mutex_;
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<config::Configuration> configuration_;
  boost::thread_group user_threads_;
  boost::thread_group io_threads_;
  std::atomic_size_t user_thread_count_ = kDefaultUserThreadCount;
  std::atomic_size_t io_thread_count_ = kDefaultIoThreadCount;

  /**
   * @brief Добавить определенное количество потоков в группу потоков, при этом каждый из потоков
   * выполняет метод run из переданного контекста (boost::asio::io_context).
   */
  static void AddThreadsToGroup(
      boost::thread_group &thread_group, size_t thread_count, boost::asio::io_context &context
  );

  TaskManagerImpl(
      std::shared_ptr<config::Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );

  /**
   * @brief Обернуть переданную задачу в специальный класс.
   */
  TaskWrapped<Task> MakeTaskWrapped(std::function<Task> task) const;
  /**
   * @brief Обернуть переданную задачу с таймером в специальный класс.
   */
  TimerTaskWrapped<Task, Clock_t> MakeTimerTaskWrapped(
      std::function<Task> task, std::unique_ptr<TimerTaskWrapped<Task, Clock_t>::Timer> timer
  ) const;
  /**
   * @brief Прочитать значение количества пользовательских потоков из конфигурации.
   */
  [[nodiscard]] size_t ReadUserThreadCount() const;
  /**
   * @brief Прочитать значение количества потоков ввода-вывода из конфигурации.
   */
  [[nodiscard]] size_t ReadIoThreadCount() const;
};

}  // namespace call_center::core::tasks

#endif  // CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_IMPL_H_
