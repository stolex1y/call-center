#include "call_detailed_record.h"

#include <utility>

using namespace std::chrono_literals;
namespace uuids = boost::uuids;

namespace call_center {

CallDetailedRecord::CallDetailedRecord(
    std::string caller_phone_number,
    std::shared_ptr<Configuration> configuration,
    OnFinish on_finish
)
    : configuration_(std::move(configuration)),
      caller_phone_number_(std::move(caller_phone_number)),
      on_finish_(std::move(on_finish)) {
  max_wait_ = WaitingDuration(ReadMaxWait());
}

void CallDetailedRecord::StartService(uuids::uuid operator_id) {
  std::lock_guard lock(mutex_);
  assert(WasArrived_() && !WasFinished_());
  operator_id_ = operator_id;
  start_service_time_ = time_point_cast<Duration>(Clock::now());
}

void CallDetailedRecord::CompleteService(CallStatus status) {
  {
    std::lock_guard lock(mutex_);
    assert(WasArrived_() && !WasFinished_());
    status_ = status;
    complete_service_time_ = time_point_cast<Duration>(Clock::now());
  }
  on_finish_(*this);
}

bool CallDetailedRecord::WasServiced() const {
  std::shared_lock lock(mutex_);
  return WasServiced_();
}

bool CallDetailedRecord::WasArrived() const {
  std::shared_lock lock(mutex_);
  return WasArrived_();
}

bool CallDetailedRecord::WasFinished() const {
  std::shared_lock lock(mutex_);
  return WasFinished_();
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetArrivalTime() const {
  std::shared_lock lock(mutex_);
  return arrival_time_;
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetServiceCompleteTime() const {
  std::shared_lock lock(mutex_);
  return complete_service_time_;
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetServiceStartTime() const {
  std::shared_lock lock(mutex_);
  return start_service_time_;
}

const std::string &CallDetailedRecord::GetCallerPhoneNumber() const {
  return caller_phone_number_;
}

std::optional<CallStatus> CallDetailedRecord::GetStatus() const {
  std::shared_lock lock(mutex_);
  return status_;
}

std::optional<uuids::uuid> CallDetailedRecord::GetOperatorId() const {
  std::shared_lock lock(mutex_);
  return operator_id_;
}

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetServiceTime() const {
  std::shared_lock lock(mutex_);
  if (WasServiced_()) {
    return *complete_service_time_ - *start_service_time_;
  } else {
    return std::nullopt;
  }
}

CallDetailedRecord::WaitingDuration CallDetailedRecord::GetMaxWait() const {
  return max_wait_;
}

bool CallDetailedRecord::IsTimeout() const {
  std::shared_lock lock(mutex_);
  if (WasArrived_())
    return Clock::now() >= timeout_point_;
  else
    return false;
}

uint64_t CallDetailedRecord::ReadMaxWait() const {
  return configuration_->GetProperty<uint64_t>(kMaxWaitKey, max_wait_.count());
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetTimeoutPoint() const {
  std::shared_lock lock(mutex_);
  return timeout_point_;
}

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetWaitTime() const {
  std::shared_lock lock(mutex_);
  if (start_service_time_ != std::nullopt) {
    return *start_service_time_ - *arrival_time_;
  }
  if (WasFinished_()) {
    return *complete_service_time_ - *arrival_time_;
  }
  return std::nullopt;
}

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetTotalTime() const {
  std::shared_lock lock(mutex_);
  if (WasFinished_()) {
    if (WasServiced_()) {
      return *GetWaitTime() + *GetServiceTime();
    } else {
      return *GetWaitTime();
    }
  } else {
    return std::nullopt;
  }
}

void CallDetailedRecord::SetArrivalTime() {
  std::lock_guard lock(mutex_);
  assert(!WasArrived_());
  arrival_time_ = time_point_cast<Duration>(Clock::now());
  timeout_point_ = *arrival_time_ + max_wait_;
}

bool CallDetailedRecord::WasArrived_() const {
  return arrival_time_ != std::nullopt;
}

bool CallDetailedRecord::WasFinished_() const {
  return status_ != std::nullopt;
}

bool CallDetailedRecord::WasServiced_() const {
  return status_ == CallStatus::kOk;
}

}  // namespace call_center
