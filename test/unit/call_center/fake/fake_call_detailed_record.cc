#include "fake_call_detailed_record.h"

#include <memory>

namespace call_center {

std::shared_ptr<FakeCallDetailedRecord> FakeCallDetailedRecord::Create(
    std::shared_ptr<const core::ClockAdapter> clock,
    std::string caller_phone_number,
    std::shared_ptr<Configuration> configuration,
    OnFinish on_finish
) {
  return std::make_shared<FakeCallDetailedRecord>(
      std::move(clock),
      std::move(caller_phone_number),
      std::move(configuration),
      std::move(on_finish)
  );
}

FakeCallDetailedRecord::FakeCallDetailedRecord(
    std::shared_ptr<const core::ClockAdapter> clock,
    std::string caller_phone_number,
    std::shared_ptr<Configuration> configuration,
    OnFinish on_finish
)
    : CallDetailedRecord(
          std::move(caller_phone_number), std::move(configuration), std::move(on_finish)
      ),
      clock_(std::move(clock)) {
}

void FakeCallDetailedRecord::StartService(boost::uuids::uuid operator_id) {
  std::lock_guard lock(mutex_);
  operator_id_ = operator_id;
  start_service_time_ = time_point_cast<Duration>(clock_->Now());
}

void FakeCallDetailedRecord::CompleteService(CallStatus status) {
  {
    std::lock_guard lock(mutex_);
    status_ = status;
    complete_service_time_ = time_point_cast<Duration>(clock_->Now());
  }
  on_finish_(*this);
}

bool FakeCallDetailedRecord::IsTimeout() const {
  std::shared_lock lock(mutex_);
  return clock_->Now() >= timeout_point_;
}

void FakeCallDetailedRecord::SetArrivalTime() {
  std::lock_guard lock(mutex_);
  arrival_time_ = std::chrono::time_point_cast<Duration, Clock>(clock_->Now());
  timeout_point_ = *arrival_time_ + max_wait_;
}

}  // namespace call_center
