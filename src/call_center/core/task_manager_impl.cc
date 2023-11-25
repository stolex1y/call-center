#include "task_manager_impl.h"

#include <boost/asio/deadline_timer.hpp>

namespace call_center::core {

namespace asio = boost::asio;

const size_t TaskManagerImpl::kDefaultUserThreadCount =
    std::max(size_t(boost::thread::hardware_concurrency()), size_t(1));
const size_t TaskManagerImpl::kDefaultIoThreadCount =
    std::max(size_t(boost::thread::hardware_concurrency()), size_t(64));

std::shared_ptr<TaskManagerImpl> TaskManagerImpl::Create(
    std::shared_ptr<Configuration> configuration,
    const std::shared_ptr<const log::LoggerProvider> &logger_provider
) {
  return std::shared_ptr<TaskManagerImpl>(
      new TaskManagerImpl(std::move(configuration), logger_provider)
  );
}

TaskManagerImpl::TaskManagerImpl(
    std::shared_ptr<Configuration> configuration,
    const std::shared_ptr<const log::LoggerProvider> &logger_provider
)
    : user_work_guard_(asio::make_work_guard(user_context_)),
      io_work_guard_(asio::make_work_guard(io_context_)),
      logger_(logger_provider->Get("TaskManagerImpl")),
      configuration_(std::move(configuration)) {
}

void TaskManagerImpl::Start() {
  if (stopped_.test()) {
    assert(!stopped_.test());
    return;
  }
  if (started_.test_and_set()) {
    return;
  }
  io_thread_count_ = ReadIoThreadCount();
  AddThreadsToGroup(io_threads_, io_thread_count_, io_context_);

  user_thread_count_ = ReadUserThreadCount();
  AddThreadsToGroup(user_threads_, user_thread_count_, user_context_);
}

void TaskManagerImpl::Stop() {
  if (stopped_.test_and_set()) {
    return;
  }
  io_work_guard_.reset();
  user_work_guard_.reset();
  io_context_.stop();
  user_context_.stop();
  Join();
}

void TaskManagerImpl::Join() {
  user_threads_.join_all();
  io_threads_.join_all();
}

asio::io_context &TaskManagerImpl::IoContext() {
  return io_context_;
}

void core::TaskManagerImpl::PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) {
  logger_->Info() << "Starting the timer of task delayed by "
                  << std::chrono::floor<std::chrono::milliseconds>(delay);

  auto timer = std::make_unique<typename tasks::TimerTaskWrapped<Task, Clock_t>::Timer>(
      user_context_, delay
  );
  timer->async_wait(MakeTimerTaskWrapped(std::move(task), std::move(timer)));
}

void core::TaskManagerImpl::PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) {
  logger_->Info() << "Starting the timer of deferred task until " << time_point;
  auto timer = std::make_unique<typename tasks::TimerTaskWrapped<Task, Clock_t>::Timer>(
      user_context_, time_point
  );
  timer->async_wait(MakeTimerTaskWrapped(std::move(task), std::move(timer)));
}

void TaskManagerImpl::PostTask(std::function<Task> task) {
  user_context_.post(MakeTaskWrapped(std::move(task)));
}

tasks::TimerTaskWrapped<TaskManager::Task, TaskManager::Clock_t>
TaskManagerImpl::MakeTimerTaskWrapped(
    std::function<Task> task,
    std::unique_ptr<typename tasks::TimerTaskWrapped<Task, Clock_t>::Timer> timer
) {
  return {std::move(task), std::move(timer), *logger_};
}

tasks::TaskWrapped<TaskManager::Task> TaskManagerImpl::MakeTaskWrapped(std::function<Task> task) {
  return {std::move(task), *logger_};
}

size_t TaskManagerImpl::ReadUserThreadCount() const {
  return configuration_->GetNumber<size_t>(kUserThreadCountKey, user_thread_count_, 1);
}

size_t TaskManagerImpl::ReadIoThreadCount() const {
  return configuration_->GetNumber<size_t>(kIoThreadCountKey, io_thread_count_, 1);
}

void TaskManagerImpl::AddThreadsToGroup(
    boost::thread_group &thread_group, size_t thread_count, boost::asio::io_context &context
) {
  for (size_t i = 0; i < thread_count; ++i) {
    thread_group.add_thread(new boost::thread([&context]() {
      context.run();
    }));
  }
}

size_t TaskManagerImpl::GetUserThreadCount() const {
  return user_thread_count_;
}

size_t TaskManagerImpl::GetIoThreadCount() const {
  return io_thread_count_;
}

}  // namespace call_center::core
