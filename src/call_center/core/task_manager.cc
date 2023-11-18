#include "task_manager.h"

#include <boost/asio/deadline_timer.hpp>

namespace call_center::core {

namespace asio = boost::asio;

TaskManager::TaskManager(const log::Sink &sink)
    : work_guard_(asio::make_work_guard(ioc_)), logger_("TaskManager", sink) {}

void TaskManager::Start() {
  {
    std::lock_guard lock(mutex_);
    if (!stopped_)
      return;
    stopped_ = false;
  }
  thread_ = std::thread([&ioc = ioc_]() { ioc.run(); });
}

void TaskManager::Stop() {
  {
    std::lock_guard lock(mutex_);
    if (stopped_)
      return;
    stopped_ = true;
  }
  work_guard_.reset();
  if (thread_.joinable())
    thread_.join();
}

void TaskManager::Join() {
  if (thread_.joinable())
    thread_.join();
}

asio::io_context &TaskManager::IoContext() {
  return ioc_;
}

}
