#ifndef CALL_CENTER_SRC_CALL_CENTER_TASKS_H_
#define CALL_CENTER_SRC_CALL_CENTER_TASKS_H_

#include <boost/asio.hpp>

#include <thread>
#include <functional>
#include <chrono>

#include "log/logger.h"

namespace call_center::core::tasks {

template<typename Task>
class TaskWrapped {
 public:
  TaskWrapped(std::function<Task> task, log::Logger &logger);

  void operator()() const;

 private:
  std::function<Task> task_;
  log::Logger &logger_;
};

template<typename Task, typename Clock>
class TimerTaskWrapped {
 public:
  using Timer = boost::asio::basic_waitable_timer<Clock>;

  TimerTaskWrapped(std::function<Task> task,
                   std::unique_ptr<Timer> timer,
                   log::Logger &logger);

  void operator()(const boost::system::error_code &error) const;

 private:
  const TaskWrapped<Task> task_;
  log::Logger &logger_;
  std::unique_ptr<Timer> timer_;
};

template<typename Task, typename Clock>
void TimerTaskWrapped<Task, Clock>::operator()(const boost::system::error_code &error) const {
  if (!error) {
    task_();
  } else {
    logger_.Error() << "System error in timer that used by the deferred task: " << error;
  }
}

template<typename Task, typename Clock>
TimerTaskWrapped<Task, Clock>::TimerTaskWrapped(std::function<Task> task,
                                                std::unique_ptr<Timer> timer,
                                                log::Logger &logger)
    : task_(std::move(task), logger), logger_(logger), timer_(std::move(timer)) {}

template<typename Task>
TaskWrapped<Task>::TaskWrapped(std::function<Task> task,
                               log::Logger &logger)
    : task_(std::move(task)), logger_(logger) {}

template<typename Task>
void TaskWrapped<Task>::operator()() const {
  try {
    logger_.Info() << "Starting execution of the user task";
    task_();
  } catch (const std::exception &ex) {
    logger_.Error() << "Unhandled std::exception in task: " << ex.what();
  } catch (...) {
    logger_.Error() << "Unknown unhandled exception in task";
  }
}

}

#endif //CALL_CENTER_SRC_CALL_CENTER_TASKS_H_
