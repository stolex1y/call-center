#include "operator.h"

namespace call_center {

Operator::Operator(core::TaskManager &task_manager, const Configuration &configuration)
    : task_manager_(task_manager), configuration_(configuration),
      min_delay_(ReadMinDelay()), max_delay_(ReadMaxDelay()),
      generator_(boost::hash_value(id_)), distribution_(ReadMinDelay(), ReadMaxDelay()) {
}

const boost::uuids::uuid &Operator::GetId() const {
  return id_;
}

Operator::Status Operator::GetStatus() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return status_;
}

void Operator::HandleCall(const std::shared_ptr<CallDetailedRecord> &call,
                          const OnFinishHandle &on_finish) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(status_ == Status::kFree);
    status_ = Status::kBusy;
  }

  boost::ignore_unused(call);
  const auto finish_handle = [op = shared_from_this(), on_finish]() mutable {
    on_finish();
    std::lock_guard<std::mutex> lock(op->mutex_);
    op->status_ = Status::kFree;
  };
  task_manager_.PostTaskDelayed<void()>(DelayDuration(GetCallDelay()), finish_handle);
}

uint64_t Operator::ReadMinDelay() const {
  return configuration_.GetProperty<uint64_t>(kMinDelayKey).value_or(kDefaultMinDelay);
}

uint64_t Operator::ReadMaxDelay() const {
  return configuration_.GetProperty<uint64_t>(kMaxDelayKey).value_or(kDefaultMaxDelay);
}

uint64_t Operator::GetCallDelay() {
  UpdateDistribution();
  return distribution_(generator_);
}

void Operator::UpdateDistribution() {
  auto new_min = ReadMinDelay();
  auto new_max = ReadMaxDelay();
  if (new_min != min_delay_ || new_max != max_delay_) {
    distribution_ = Distribution(min_delay_, max_delay_);
  }
}

std::shared_ptr<Operator> Operator::Create(core::TaskManager &task_manager, const Configuration &configuration) {
  return std::shared_ptr<Operator>(new Operator(task_manager, configuration));
}

} // call_center