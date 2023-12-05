#include "fake_service_loader.h"

#include "fake_call_detailed_record.h"

namespace call_center::test {

FakeServiceLoader::FakeServiceLoader(
    std::shared_ptr<tasks::TaskManager> task_manager,
    std::shared_ptr<CallCenter> call_center,
    CallProvider call_provider
)
    : task_manager_(std::move(task_manager)),
      call_center_(std::move(call_center)),
      generator_(std::random_device()()),
      call_provider_(std::move(call_provider)) {
}

std::shared_ptr<FakeServiceLoader> FakeServiceLoader::Create(
    std::shared_ptr<tasks::TaskManager> task_manager,
    std::shared_ptr<CallCenter> call_center,
    CallProvider call_provider
) {
  return std::shared_ptr<FakeServiceLoader>(new FakeServiceLoader(
      std::move(task_manager), std::move(call_center), std::move(call_provider)
  ));
}

void FakeServiceLoader::StartLoading(const double lambda) {
  distribution_ = Distribution(lambda);
  SchedulePostCall();
}

void FakeServiceLoader::SchedulePostCall() {
  const auto delay = DelayDuration(distribution_(generator_));
  task_manager_->PostTaskDelayed(delay, [loader = shared_from_this()] {
    loader->call_center_->PushCall(loader->call_provider_());
    loader->SchedulePostCall();
  });
}

}  // namespace call_center::test
