#include "call_detailed_record.h"

#include <boost/uuid/uuid_generators.hpp>
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
      receipt_time_(time_point_cast<Duration>(Clock::now())),
      id_(uuids::random_generator_mt19937()()),
      caller_phone_number_(std::move(caller_phone_number)),
      on_finish_(std::move(on_finish)) {
  end_processing_time_ = TimePoint{Duration(0)};
  start_processing_time_ = TimePoint{Duration(0)};
  max_wait_ = WaitingDuration(ReadMaxWait());
  timeout_point_ = receipt_time_ + max_wait_;
}

void CallDetailedRecord::StartProcessing(boost::uuids::uuid operator_id) {
  std::lock_guard lock(mutex_);
  operator_id_ = operator_id;
  start_processing_time_ = time_point_cast<Duration>(Clock::now());
}

void CallDetailedRecord::FinishProcessing(CallStatus status) {
  {
    std::lock_guard lock(mutex_);
    status_ = status;
    end_processing_time_ = time_point_cast<Duration>(Clock::now());
  }
  on_finish_(*this);
}

bool CallDetailedRecord::WasProcessed() const {
  std::shared_lock lock(mutex_);
  return status_ == CallStatus::kOk;
}

bool CallDetailedRecord::WasFinished() const {
  std::shared_lock lock(mutex_);
  return status_ != std::nullopt;
}

CallDetailedRecord::TimePoint CallDetailedRecord::GetReceiptTime() const {
  return receipt_time_;
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetEndProcessingTime() const {
  std::shared_lock lock(mutex_);
  return end_processing_time_;
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetStartProcessingTime() const {
  std::shared_lock lock(mutex_);
  return start_processing_time_;
}

uuids::uuid CallDetailedRecord::GetId() const {
  return id_;
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

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetProcessingDuration() const {
  std::shared_lock lock(mutex_);
  if (WasProcessed()) {
    return *end_processing_time_ - *start_processing_time_;
  } else {
    return std::nullopt;
  }
}

CallDetailedRecord::WaitingDuration CallDetailedRecord::GetMaxWait() const {
  return max_wait_;
}

bool CallDetailedRecord::IsTimeout() const {
  return (Clock::now() - GetReceiptTime()) >= max_wait_;
}

uint64_t CallDetailedRecord::ReadMaxWait() const {
  return configuration_->GetProperty<uint64_t>(kMaxWaitKey_, max_wait_.count());
}

CallDetailedRecord::TimePoint CallDetailedRecord::GetTimeoutPoint() const {
  return timeout_point_;
}

bool CallDetailedRecord::operator==(const CallDetailedRecord &other) const {
  return caller_phone_number_ == other.caller_phone_number_;
}

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetWaitingDuration() const {
  std::shared_lock lock(mutex_);
  if (WasFinished()) {
    if (WasProcessed()) {
      return *start_processing_time_ - receipt_time_;
    } else {
      return *end_processing_time_ - receipt_time_;
    }
  } else {
    return std::nullopt;
  }
}

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetTotalTime() const {
  std::shared_lock lock(mutex_);
  if (WasFinished()) {
    if (WasProcessed()) {
      return *GetWaitingDuration() + *GetProcessingDuration();
    } else {
      return *GetWaitingDuration();
    }
  } else {
    return std::nullopt;
  }
}

}  // namespace call_center
