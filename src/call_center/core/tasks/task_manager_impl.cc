#include "task_manager_impl.h"

#include <boost/asio/deadline_timer.hpp>

namespace call_center::core::tasks {

namespace asio = boost::asio;

const size_t TaskManagerImpl::kDefaultUserThreadCount =
    std::max(static_cast<size_t>(boost::thread::hardware_concurrency()), static_cast<size_t>(1));
const size_t TaskManagerImpl::kDefaultIoThreadCount =
    std::max(static_cast<size_t>(boost::thread::hardware_concurrency()), static_cast<size_t>(64));

std::shared_ptr<TaskManagerImpl> TaskManagerImpl::Create(
    std::shared_ptr<config::Configuration> configuration, const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<TaskManagerImpl>(
      new TaskManagerImpl(std::move(configuration), logger_provider)
  );
}

TaskManagerImpl::TaskManagerImpl(
    std::shared_ptr<config::Configuration> configuration, const log::LoggerProvider &logger_provider
)
    : user_work_guard_(make_work_guard(user_context_)),
      io_work_guard_(make_work_guard(io_context_)),
      logger_(logger_provider.Get("TaskManagerImpl")),
      configuration_(std::move(configuration)) {
}

TaskManagerImpl::~TaskManagerImpl() {
  Stop();
}

void TaskManagerImpl::Start() {
  std::lock_guard lock(start_mutex_);
  if (stopped_) {
    logger_->Warning() << "Start after stop isn't working!";
    return;
  }
  if (started_) {
    return;
  }
  started_ = true;

  io_thread_count_ = ReadIoThreadCount();
  AddThreadsToGroup(io_threads_, io_thread_count_, io_context_);

  user_thread_count_ = ReadUserThreadCount();
  AddThreadsToGroup(user_threads_, user_thread_count_, user_context_);
}

void TaskManagerImpl::Stop() {
  {
    std::lock_guard lock(start_mutex_);
    if (stopped_) {
      return;
    }
    stopped_ = true;

    io_work_guard_.reset();
    user_work_guard_.reset();
    io_context_.stop();
    user_context_.stop();
  }
  Join();
}

void TaskManagerImpl::Join() {
  user_threads_.join_all();
  io_threads_.join_all();
}

asio::io_context &TaskManagerImpl::IoContext() {
  return io_context_;
}

void TaskManagerImpl::PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) {
  logger_->Info() << "Starting the timer of task delayed by "
                  << std::chrono::floor<std::chrono::milliseconds>(delay);

  auto timer = std::make_unique<TimerTaskWrapped<Task, Clock_t>::Timer>(user_context_, delay);
  timer->async_wait(MakeTimerTaskWrapped(std::move(task), std::move(timer)));
}

void TaskManagerImpl::PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) {
  logger_->Info() << "Starting the timer of deferred task until " << time_point;
  auto timer = std::make_unique<TimerTaskWrapped<Task, Clock_t>::Timer>(user_context_, time_point);
  timer->async_wait(MakeTimerTaskWrapped(std::move(task), std::move(timer)));
}

void TaskManagerImpl::PostTask(std::function<Task> task) {
  user_context_.post(MakeTaskWrapped(std::move(task)));
}

TimerTaskWrapped<TaskManager::Task, TaskManager::Clock_t> TaskManagerImpl::MakeTimerTaskWrapped(
    std::function<Task> task, std::unique_ptr<TimerTaskWrapped<Task, Clock_t>::Timer> timer
) const {
  return {std::move(task), std::move(timer), *logger_};
}

TaskWrapped<TaskManager::Task> TaskManagerImpl::MakeTaskWrapped(std::function<Task> task) const {
  return {std::move(task), *logger_};
}

size_t TaskManagerImpl::ReadUserThreadCount() const {
  return configuration_->GetNumber<size_t>(kUserThreadCountKey, user_thread_count_, 1);
}

size_t TaskManagerImpl::ReadIoThreadCount() const {
  return configuration_->GetNumber<size_t>(kIoThreadCountKey, io_thread_count_, 1);
}

void TaskManagerImpl::AddThreadsToGroup(
    boost::thread_group &thread_group, const size_t thread_count, asio::io_context &context
) {
  for (size_t i = 0; i < thread_count; ++i) {
    thread_group.add_thread(new boost::thread([&context] {
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

}  // namespace call_center::core::tasks
