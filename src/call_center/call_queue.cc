#include "call_queue.h"

#include <boost/functional/hash.hpp>

namespace call_center {

CallQueue::CallQueue(
    std::shared_ptr<Configuration> configuration,
    const std::shared_ptr<const log::LoggerProvider> &logger_provider
)
    : logger_(logger_provider->Get("CallQueue")), configuration_(std::move(configuration)) {
  capacity_ = ReadCapacity();
}

CallQueue::CallPtr CallQueue::PopFromQueue() {
  std::lock_guard<std::mutex> lock(queue_mutex_);

  if (in_receipt_order_.empty())
    return nullptr;

  auto result = *in_receipt_order_.begin();
  EraseFromQueue(result);
  InsertToProcessing(result);
  return result;
}

CallQueue::PushResult CallQueue::PushToQueue(const CallPtr &call) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  UpdateCapacity();

  if (Contains(call)) {
    return PushResult::kAlreadyInQueue;
  }
  if (in_receipt_order_.size() >= capacity_) {
    return PushResult::kOverload;
  }
  InsertToQueue(call);
  return PushResult::kOk;
}

bool CallQueue::QueueIsEmpty() const {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  return in_receipt_order_.empty();
}

CallQueue::CallPtr CallQueue::EraseTimeoutCallFromQueue() {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  if (in_receipt_order_.empty())
    return nullptr;

  auto least = *in_timout_point_order_.begin();
  if (least->IsTimeout()) {
    EraseFromQueue(least);
    return least;
  } else {
    return nullptr;
  }
}

CallQueue::CallPtr CallQueue::GetMinTimeoutCallInQueue() const {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  if (in_receipt_order_.empty())
    return nullptr;

  return *in_timout_point_order_.begin();
}

void CallQueue::EraseFromProcessing(const CallPtr &call) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  in_processing_.erase(call);
}

bool CallQueue::InsertToProcessing(const CallPtr &call) {
  std::lock_guard<std::mutex> lock(queue_mutex_);

  if (Contains(call))
    return false;

  in_processing_.emplace(call);
  return true;
}

size_t CallQueue::ReadCapacity() const {
  return configuration_->GetProperty(kCapacityKey_, capacity_);
}

void CallQueue::UpdateCapacity() {
  const size_t new_capacity = ReadCapacity();
  if (new_capacity != capacity_) {
    capacity_ = new_capacity;
  }
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
