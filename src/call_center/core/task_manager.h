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
  using Task = void();

  TaskManager() = default;
  TaskManager(const TaskManager &other) = delete;
  TaskManager &operator=(const TaskManager &other) = delete;
  virtual ~TaskManager() = default;

  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual void Join() = 0;
  virtual boost::asio::io_context &IoContext() = 0;

  virtual void PostTask(std::function<Task> task) = 0;

  template <typename TimePoint>
  void PostTaskAt(const TimePoint &time_point, const std::function<Task> &task);

  template <typename Duration>
  void PostTaskDelayed(const Duration &delay, const std::function<Task> &task);

 protected:
  using Clock_t = std::chrono::utc_clock;
  using Duration_t = std::chrono::milliseconds;
  using TimePoint_t = std::chrono::time_point<Clock_t, Duration_t>;

  virtual void PostTaskDelayedImpl(Duration_t delay, std::function<Task> task) = 0;
  virtual void PostTaskAtImpl(TimePoint_t time_point, std::function<Task> task) = 0;
};

template <typename Duration>
void TaskManager::PostTaskDelayed(const Duration &delay, const std::function<Task> &task) {
  PostTaskDelayedImpl(std::chrono::duration_cast<Duration_t>(delay), task);
}

template <typename TimePoint>
void TaskManager::PostTaskAt(const TimePoint &time_point, const std::function<Task> &task) {
  PostTaskAtImpl(
      std::chrono::time_point_cast<Duration_t, Clock_t, typename TimePoint::duration>(time_point),
      task
  );
}

}  // namespace call_center::core

#endif  // CALL_CENTER_SRC_CALL_CENTER_TASK_MANAGER_H_
