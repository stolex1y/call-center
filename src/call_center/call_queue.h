#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_QUEUE_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_QUEUE_H_

#include <functional>
#include <set>
#include <unordered_set>

#include "call_detailed_record.h"
#include "configuration.h"
#include "core/containers/queue.h"

namespace call_center {

class CallQueue {
 public:
  using CallPtr = std::shared_ptr<CallDetailedRecord>;

  enum class PushResult { kOk, kAlreadyInQueue, kOverload };

  CallQueue(
      std::shared_ptr<Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );
  CallQueue(const CallQueue &other) = delete;
  CallQueue &operator=(const CallQueue &other) = delete;

  CallPtr PopFromQueue();
  PushResult PushToQueue(const CallPtr &call);
  [[nodiscard]] bool QueueIsEmpty() const;
  CallPtr EraseTimeoutCallFromQueue();
  CallPtr GetMinTimeoutCallInQueue() const;
  void EraseFromProcessing(const CallPtr &call);
  bool InsertToProcessing(const CallPtr &call);
  [[nodiscard]] bool Contains(const CallPtr &call) const;

 private:
  struct CallEquals {
    bool operator()(const CallPtr &first, const CallPtr &second) const;
  };

  struct CallHash {
    size_t operator()(const CallPtr &call) const;
  };

  struct ReceiptOrder {
    bool operator()(const CallPtr &first, const CallPtr &second) const;
  };

  struct TimeoutPointOrder {
    bool operator()(const CallPtr &first, const CallPtr &second) const;
  };

  static constexpr const auto kCapacityKey_ = "queue_capacity";
  static constexpr const size_t kDefaultCapacity_ = 10;

  std::unordered_set<CallPtr, CallHash, CallEquals> in_processing_;
  // TODO extract to its own class
  std::multiset<CallPtr, TimeoutPointOrder> in_timout_point_order_;
  std::multiset<CallPtr, ReceiptOrder> in_receipt_order_;
  mutable std::mutex queue_mutex_;
  std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<Configuration> configuration_;
  size_t capacity_ = kDefaultCapacity_;

  [[nodiscard]] size_t ReadCapacity() const;
  void UpdateCapacity();
  void EraseFromQueue(const CallPtr &call);
  void InsertToQueue(const CallPtr &call);

  template <typename Cmp>
  static void EraseCallFromMultiset(
      std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call
  );

  template <typename Cmp>
  static bool MultisetContainsCall(
      const std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call
  );
};

template <typename Cmp>
void CallQueue::EraseCallFromMultiset(
    std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call
) {
  CallEquals equals;
  for (auto [begin, end] = multiset.equal_range(call); begin != end; ++begin) {
    if (equals(*begin, call)) {
      multiset.erase(begin);
      break;
    }
  }
}

template <typename Cmp>
bool CallQueue::MultisetContainsCall(
    const std::multiset<CallPtr, Cmp> &multiset, const CallPtr &call
) {
  CallEquals equals;
  for (auto [begin, end] = multiset.equal_range(call); begin != end; ++begin) {
    if (equals(*begin, call)) {
      return true;
    }
  }
  return false;
}

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_QUEUE_H_
