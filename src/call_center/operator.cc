#include "operator.h"

#include <boost/uuid/uuid_io.hpp>

namespace call_center {

using namespace core::tasks;

std::shared_ptr<Operator> Operator::Create(
    std::shared_ptr<TaskManager> task_manager,
    std::shared_ptr<Configuration> configuration,
    const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<Operator>(
      new Operator(std::move(task_manager), std::move(configuration), logger_provider)
  );
}

Operator::Operator(
    std::shared_ptr<TaskManager> task_manager,
    std::shared_ptr<Configuration> configuration,
    const log::LoggerProvider &logger_provider
)
    : task_manager_(std::move(task_manager)),
      configuration_(std::move(configuration)),
      generator_(boost::hash_value(id_)),
      logger_(logger_provider.Get("Operator (" + boost::uuids::to_string(id_) + ")")) {
  InitDistributionParameters();
}

bool Operator::IsFree() const {
  return status_ == Status::kFree;
}

bool Operator::IsBusy() const {
  return status_ == Status::kBusy;
}

void Operator::HandleCall(
    const std::shared_ptr<CallDetailedRecord> &call, const OnFinishHandle &on_finish
) {
  assert(status_ == Status::kFree);
  status_ = Status::kBusy;

  const auto finish_handle = [op = shared_from_this(), on_finish]() mutable {
    op->status_ = Status::kFree;
    on_finish();
  };
  const auto delay = GetCallDelay();

  logger_->Info() << "Handle call '" << boost::uuids::to_string(call->GetId()) << "' for " << delay;

  task_manager_->PostTaskDelayed(delay, finish_handle);
}

void Operator::UpdateDistributionParameters() {
  auto [new_min, new_max] = ReadMinMax();
  if (new_min != min_delay_ || new_max != max_delay_) {
    min_delay_ = new_min;
    max_delay_ = new_max;
    distribution_ = Distribution(min_delay_, max_delay_);
    logger_->Debug() << "Update distribution parameters: " << min_delay_ << ", " << max_delay_;
  }
}

std::pair<uint64_t, uint64_t> Operator::ReadMinMax() const {
  const uint64_t new_min = configuration_->GetProperty(kMinDelayKey, min_delay_);
  const uint64_t new_max = configuration_->GetNumber(kMaxDelayKey, max_delay_, new_min);
  return {new_min, new_max};
}

void Operator::InitDistributionParameters() {
  std::tie(min_delay_, max_delay_) = ReadMinMax();
  distribution_ = Distribution(min_delay_, max_delay_);
}

Operator::DelayDuration Operator::GetCallDelay() {
  UpdateDistributionParameters();
  return DelayDuration(distribution_(generator_));
}

}  // namespace call_center
