#include "call_queue.h"

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace call_center {

CallQueue::CallQueue(
    std::shared_ptr<Configuration> configuration, const log::LoggerProvider &logger_provider
)
    : logger_(logger_provider.Get("CallQueue")), configuration_(std::move(configuration)) {
  UpdateCapacity();
}

CallQueue::CallPtr CallQueue::PopFromQueue() {
  std::lock_guard lock(queue_mutex_);

  if (in_receipt_order_.empty())
    return nullptr;

  auto result = *in_receipt_order_.begin();
  EraseFromQueue(result);
  logger_->Debug() << "Pop call " << result->GetId() << " from queue";
  return result;
}

CallQueue::PushResult CallQueue::PushToQueue(const CallPtr &call) {
  std::lock_guard lock(queue_mutex_);
  UpdateCapacity();

  if (Contains(call)) {
    logger_->Debug() << "Couldn't add call " << call->GetId() << ": already in queue";
    return PushResult::kAlreadyInQueue;
  }
  if (in_receipt_order_.size() >= capacity_) {
    logger_->Debug() << "Couldn't add call " << call->GetId() << ": overload";
    return PushResult::kOverload;
  }
  InsertToQueue(call);
  logger_->Debug() << "Add call " << call->GetId() << " to queue";
  return PushResult::kOk;
}

bool CallQueue::QueueIsEmpty() const {
  std::shared_lock lock(queue_mutex_);
  return in_receipt_order_.empty();
}

CallQueue::CallPtr CallQueue::EraseTimeoutCallFromQueue() {
  std::lock_guard lock(queue_mutex_);
  if (in_receipt_order_.empty())
    return nullptr;

  auto least = *in_timout_point_order_.begin();
  if (least->IsTimeout()) {
    EraseFromQueue(least);
    logger_->Debug() << "Erase timeout call " << least->GetId();
    return least;
  } else {
    return nullptr;
  }
}

CallQueue::CallPtr CallQueue::GetMinTimeoutCallInQueue() const {
  std::shared_lock lock(queue_mutex_);
  if (in_receipt_order_.empty())
    return nullptr;

  logger_->Debug() << "Min timeout in queue: "
                   << *(*in_timout_point_order_.begin())->GetTimeoutPoint();
  return *in_timout_point_order_.begin();
}

void CallQueue::EraseFromProcessing(const CallPtr &call) {
  std::lock_guard lock(queue_mutex_);
  logger_->Debug() << "Remove call " << call->GetId() << " from processing set";
  in_processing_.erase(call);
}

size_t CallQueue::GetSize() const {
  std::shared_lock lock(queue_mutex_);
  return in_receipt_order_.size();
}

size_t CallQueue::GetCapacity() const {
  std::shared_lock lock(queue_mutex_);
  return capacity_;
}

bool CallQueue::InsertToProcessing(const CallPtr &call) {
  std::lock_guard lock(queue_mutex_);

  if (Contains(call))
    return false;

  in_processing_.emplace(call);
  logger_->Debug() << "Add call " << call->GetId() << " to processing set";
  return true;
}

void CallQueue::UpdateCapacity() {
  capacity_ = configuration_->GetProperty(kCapacityKey_, capacity_);
}

void CallQueue::EraseFromQueue(const CallPtr &call) {
  EraseCallFromMultiset(in_receipt_order_, call);
  EraseCallFromMultiset(in_timout_point_order_, call);
}

void CallQueue::InsertToQueue(const CallPtr &call) {
  in_receipt_order_.emplace(call);
  in_timout_point_order_.emplace(call);
}

bool CallQueue::Contains(const CallPtr &call) const {
  return MultisetContainsCall(in_receipt_order_, call) || in_processing_.contains(call);
}

bool CallQueue::CallEquals::operator()(const CallPtr &first, const CallPtr &second) const {
  if ((first == nullptr) ^ (second == nullptr))
    return false;

  if (first == nullptr)
    return true;

  return *first == *second;
}

size_t CallQueue::CallHash::operator()(const CallPtr &call) const {
  assert(call);
  return std::hash<std::string>()(call->GetCallerPhoneNumber());
}

bool CallQueue::ReceiptOrder::operator()(const CallPtr &first, const CallPtr &second) const {
  return first->GetReceiptTime() < second->GetReceiptTime();
}

bool CallQueue::TimeoutPointOrder::operator()(const CallPtr &first, const CallPtr &second) const {
  return first->GetTimeoutPoint() < second->GetTimeoutPoint();
}

}  // namespace call_center
