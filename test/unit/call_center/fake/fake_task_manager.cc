#include "fake_task_manager.h"

#include <boost/thread/detail/thread.hpp>
#include <stdexcept>

namespace call_center::core::tasks::test {

namespace asio = boost::asio;

const size_t FakeTaskManager::kThreadCount =
    std::max(static_cast<int>(boost::thread::hardware_concurrency() / 2), 1);

std::shared_ptr<FakeTaskManager> FakeTaskManager::Create(const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<FakeTaskManager>(new FakeTaskManager(logger_provider));
}

FakeTaskManager::FakeTaskManager(const log::LoggerProvider &logger_provider)
    : clock_(std::make_shared<FakeClock>()), logger_(logger_provider.Get("TaskManagerImpl")) {
}

void FakeTaskManager::Start() {
  std::lock_guard lock(start_mutex_);
  if (stopped_) {
    logger_->Warning() << "Start after stop isn't working!";
    return;
  }
  if (started_) {
    return;
  }
  started_ = true;
  for (size_t i = 0; i < kThreadCount; ++i) {
    threads_.add_thread(new boost::thread([this]() {
      HandleTasks();
    }));
  }
}

void FakeTaskManager::Stop() {
  {
    std::lock_guard lock(start_mutex_);
    if (stopped_) {
      return;
    }
    stopped_ = true;
  }
  ClearTasks();
  Join();
}

void FakeTaskManager::Join() {
  threads_.join_all();
}

asio::io_context &FakeTaskManager::IoContext() {
  throw std::runtime_error("Not implemented!");
}

void FakeTaskManager::PostTask(std::function<Task> task) {
  if (IsStopped()) {
    return;
  }
  AddTask(clock_->Now(), std::move(task));
}

void FakeTaskManager::PostTaskDelayedImpl(const Duration_t delay, std::function<Task> task) {
  if (IsStopped()) {
    return;
  }
  AddTask(clock_->Now() + delay, std::move(task));
}

void FakeTaskManager::PostTaskAtImpl(const TimePoint_t time_point, std::function<Task> task) {
  if (IsStopped()) {
    return;
  }
  AddTask(
      std::chrono::time_point_cast<FakeClock::Duration, FakeClock::Clock>(time_point),
      std::move(task)
  );
}

TaskWrapped<TaskManager::Task> FakeTaskManager::MakeTaskWrapped(std::function<Task> task) const {
  return {std::move(task), *logger_};
}

void FakeTaskManager::AdvanceTime(const Duration_t duration) {
  std::lock_guard clock_advance_lock(advance_mutex_);
  const auto target_time = clock_->Now() + duration;
  std::shared_lock tasks_lock(tasks_mutex_);
  while (clock_->Now() < target_time) {
    if (tasks_.empty()) {
      tasks_lock.unlock();
      clock_->AdvanceTo(target_time);
      return;
    }
    clock_->AdvanceTo(std::min(target_time, tasks_.begin()->first));
    has_tasks_.notify_all();
    done_tasks_.wait(tasks_lock, [this] {
      return !HasTasks();
    });
  }
}

void FakeTaskManager::ClearTasks() {
  std::lock_guard lock(tasks_mutex_);
  tasks_.clear();
  has_tasks_.notify_all();
}

void FakeTaskManager::AddTask(FakeClock::TimePoint time_point, std::function<Task> task) {
  std::lock_guard lock(tasks_mutex_);
  tasks_.emplace(time_point, MakeTaskWrapped(std::move(task)));
  if (time_point == clock_->Now()) {
    has_tasks_.notify_one();
  }
}

void FakeTaskManager::HandleTasks() {
  std::shared_lock tasks_lock(tasks_mutex_);
  while (true) {
    if (IsStopped()) {
      return;
    }

    has_tasks_.wait(tasks_lock, [this] {
      return HasTasks() || IsStopped();
    });
    tasks_lock.unlock();

    HandleFirstTask();

    tasks_lock.lock();
    if (!HasTasks() && handle_count_ == 0) {
      done_tasks_.notify_all();
    }
  }
}

void FakeTaskManager::HandleFirstTask() {
  std::unique_lock tasks_lock(tasks_mutex_);
  if (!HasTasks()) {
    return;
  }
  ++handle_count_;
  auto [time_point, task] = *tasks_.begin();
  tasks_.erase(tasks_.begin());
  tasks_lock.unlock();

  task();
  --handle_count_;
}

bool FakeTaskManager::HasTasks() const {
  if (tasks_.empty()) {
    return false;
  }
  return tasks_.begin()->first <= clock_->Now();
}

std::shared_ptr<const ClockAdapter> FakeTaskManager::GetClock() const {
  return clock_;
}

bool FakeTaskManager::IsStopped() const {
  std::shared_lock lock(start_mutex_);
  return stopped_;
}
}  // namespace call_center::core::tasks::test
