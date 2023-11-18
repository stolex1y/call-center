#include "call_detailed_record.h"

#include <boost/uuid/uuid_generators.hpp>
#include <utility>

using namespace std::chrono_literals;
namespace uuids = boost::uuids;

namespace call_center {

CallDetailedRecord::CallDetailedRecord(std::string caller_phone_number,
                                       const Configuration &configuration,
                                       OnFinish on_finish)
    : configuration_(configuration),
      receipt_time_(time_point_cast<Duration>(Clock::now())),
      max_wait_(std::chrono::seconds(ReadMaxWait())),
      timeout_point_(receipt_time_ + max_wait_),
      id_(uuids::random_generator_mt19937()()),
      caller_phone_number_(std::move(caller_phone_number)),
      on_finish_(std::move(on_finish)) {
  end_processing_time_ = TimePoint{Duration(0)};
  start_processing_time_ = TimePoint{Duration(0)};
}

void CallDetailedRecord::StartProcessing() {
  start_processing_time_ = time_point_cast<Duration>(Clock::now());
}

void CallDetailedRecord::FinishProcessing() {
  end_processing_time_ = time_point_cast<Duration>(Clock::now());
  on_finish_(*this);
}

bool CallDetailedRecord::WasProcessed() const {
  return end_processing_time_ > start_processing_time_ &&
      start_processing_time_ > receipt_time_;
}

const CallDetailedRecord::TimePoint &CallDetailedRecord::GetReceiptTime() const {
  return receipt_time_;
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetEndProcessingTime() const {
  return end_processing_time_;
}

std::optional<CallDetailedRecord::TimePoint> CallDetailedRecord::GetStartProcessingTime() const {
  return start_processing_time_;
}

const uuids::uuid &CallDetailedRecord::GetId() const {
  return id_;
}

const std::string &CallDetailedRecord::GetCallerPhoneNumber() const {
  return caller_phone_number_;
}

CallStatus CallDetailedRecord::GetStatus() const {
  return status_;
}

std::optional<uuids::uuid> CallDetailedRecord::GetOperatorId() const {
  return operator_id_;
}

std::optional<CallDetailedRecord::Duration> CallDetailedRecord::GetProcessingDuration() const {
  return end_processing_time_ - start_processing_time_;
}

void CallDetailedRecord::SetOperatorId(boost::uuids::uuid uuid) {
  operator_id_ = uuid;
}

void CallDetailedRecord::SetStatus(CallStatus status) {
  status_ = status;
}

const CallDetailedRecord::Duration &CallDetailedRecord::GetMaxWait() const {
  return max_wait_;
}

bool CallDetailedRecord::IsTimeout() const {
  return Clock::now() - GetReceiptTime() >= max_wait_;
}

uint64_t CallDetailedRecord::ReadMaxWait() const {
  return configuration_.GetProperty<uint64_t>(kMaxWaitKey).value_or(kDefaultMaxWait);
}

const CallDetailedRecord::TimePoint &CallDetailedRecord::GetTimeoutPoint() const {
  return timeout_point_;
}

bool CallDetailedRecord::operator==(const CallDetailedRecord &other) const {
  return caller_phone_number_ == other.caller_phone_number_;
}

} // call_center