#ifndef FAKE_SERVICE_LOADER_H
#define FAKE_SERVICE_LOADER_H

#include <random>

#include "call_center.h"
#include "fake_task_manager.h"

namespace call_center {

class FakeServiceLoader : public std::enable_shared_from_this<FakeServiceLoader> {
 public:
  using CallProvider = std::function<std::shared_ptr<CallDetailedRecord>()>;

  static std::shared_ptr<FakeServiceLoader> Create(
      std::shared_ptr<core::FakeTaskManager> task_manager,
      std::shared_ptr<CallCenter> call_center,
      CallProvider call_provider
  );

  void StartLoading(double lambda);

 private:
  using Distribution = std::exponential_distribution<double>;
  using Generator = std::mt19937_64;
  using DelayDuration = std::chrono::duration<double>;

  std::shared_ptr<core::FakeTaskManager> task_manager_;
  std::shared_ptr<CallCenter> call_center_;
  Generator generator_;
  Distribution distribution_;
  CallProvider call_provider_;

  FakeServiceLoader(
      std::shared_ptr<core::FakeTaskManager> task_manager,
      std::shared_ptr<CallCenter> call_center,
      CallProvider call_provider
  );

  void SchedulePostCall();
};

}  // namespace call_center

#endif  // FAKE_SERVICE_LOADER_H
