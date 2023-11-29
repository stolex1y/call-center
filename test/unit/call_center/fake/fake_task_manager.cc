#include "fake_task_manager.h"

#include <stdexcept>

namespace call_center::core {

namespace asio = boost::asio;

const size_t FakeTaskManager::kThreadCount = boost::thread::hardware_concurrency();

std::shared_ptr<FakeTaskManager> FakeTaskManager::Create(const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<FakeTaskManager>(new FakeTaskManager(logger_provider));
}

FakeTaskManager::FakeTaskManager(const log::LoggerProvider &logger_provider)
  : logger_(logger_provider.Get("TaskManagerImpl")), clock_(std::make_shared<FakeClock>()) {
}

void FakeTaskManager::Start() {
  bool was_stopped = true;
  if (!stopped_.compare_exchange_strong(was_stopped, false))
    return;
  for (size_t i = 0; i < kThreadCount; ++i) {
    threads_.add_thread(new boost::thread([this]() {
      HandleTasks();
    }));
  }
}

void FakeTaskManager::Stop() {
  bool was_stopped = false;
  if (!stopped_.compare_exchange_strong(was_stopped, true))
    return;
  has_tasks_.notify_all();
  Join();
}

void FakeTaskManager::Join() {
  threads_.join_all();
}

boost::asio::io_context &FakeTaskManager::IoContext() {
  throw std::runtime_error("Not implemented!");
}

void FakeTaskManager::PostTask(std::function<Task> task) {
  AddTask(clock_->Now(), std::move(task));
  has_tasks_.notify_one();
}

void FakeTaskManager::PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) {
  if (delay == Duration_t(0)) {
    PostTask(std::move(task));
  } else {
    AddTask(clock_->Now() + delay, std::move(task));
  }
}

void FakeTaskManager::PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) {
  if (time_point == clock_->Now()) {
    PostTask(std::move(task));
  } else {
    AddTask(
      std::chrono::time_point_cast<FakeClock::Duration, FakeClock::Clock_t>(time_point),
      std::move(task)
    );
  }
}

tasks::TaskWrapped<TaskManager::Task> FakeTaskManager::MakeTaskWrapped(std::function<Task> task) {
  return {std::move(task), *logger_};
}

void FakeTaskManager::AdvanceTime(Duration_t duration) {
  std::lock_guard clock_advance_lock(advance_mutex_);
  const auto target_time = clock_->Now() + duration;
  std::shared_lock tasks_lock(tasks_mutex_);
  while (clock_->Now() < target_time) {
    if (tasks_.empty()) {
      tasks_lock.unlock();
      clock_->AdvanceTo(target_time);
      return;
    } else {
      clock_->AdvanceTo(std::min(target_time, tasks_.begin()->first));
      has_tasks_.notify_all();
      done_tasks_.wait(tasks_lock, [this]() {
        return !HasTasks();
      });
    }
  }
}

void FakeTaskManager::ClearTasks() {
  std::lock_guard lock(tasks_mutex_);
  tasks_.clear();
}

void FakeTaskManager::AddTask(FakeClock::TimePoint time_point, std::function<Task> task) {
  std::lock_guard lock(tasks_mutex_);
  tasks_.emplace(time_point, MakeTaskWrapped(std::move(task)));
}

void FakeTaskManager::HandleTasks() {
  std::shared_lock tasks_lock(tasks_mutex_, std::defer_lock);
  while (true) {
    if (stopped_) {
      return;
    }

    tasks_lock.lock();
    has_tasks_.wait(tasks_lock, [this]() {
      return HasTasks() || stopped_;
    });
    tasks_lock.unlock();

    HandleFirstTask();

    tasks_lock.lock();
    if (!HasTasks() && handle_count_ == 0) {
      done_tasks_.notify_all();
    }
    tasks_lock.unlock();
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

}  // namespace call_center::core
