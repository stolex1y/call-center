#include "fake_call_detailed_record.h"

#include <memory>

namespace call_center {

std::shared_ptr<FakeCallDetailedRecord> FakeCallDetailedRecord::Create(
    const ClockInterface &clock,
    std::string caller_phone_number,
    std::shared_ptr<Configuration> configuration,
    CallDetailedRecord::OnFinish on_finish
) {
  return std::make_shared<FakeCallDetailedRecord>(
      clock, std::move(caller_phone_number), std::move(configuration), std::move(on_finish)
  );
}

FakeCallDetailedRecord::FakeCallDetailedRecord(
    const ClockInterface &clock,
    std::string caller_phone_number,
    std::shared_ptr<Configuration> configuration,
    CallDetailedRecord::OnFinish on_finish
)
    : CallDetailedRecord(
          std::move(caller_phone_number), std::move(configuration), std::move(on_finish)
      ),
      clock_(clock) {
}

void FakeCallDetailedRecord::StartProcessing(boost::uuids::uuid operator_id) {
  std::lock_guard lock(mutex_);
  operator_id_ = operator_id;
  start_processing_time_ = time_point_cast<Duration>(clock_.Now());
}

void FakeCallDetailedRecord::FinishProcessing(CallStatus status) {
  {
    std::lock_guard lock(mutex_);
    status_ = status;
    end_processing_time_ = time_point_cast<Duration>(clock_.Now());
  }
  on_finish_(*this);
}

bool FakeCallDetailedRecord::IsTimeout() const {
  std::shared_lock lock(mutex_);
  return clock_.Now() >= timeout_point_;
}

void FakeCallDetailedRecord::SetReceiptTime() {
  std::lock_guard lock(mutex_);
  receipt_time_ = clock_.Now();
  timeout_point_ = *receipt_time_ + max_wait_;
}

}  // namespace call_center