#ifndef CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_QUEUE_H_
#define CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_QUEUE_H_

#include <optional>
#include <queue>
#include <unordered_set>

#include "core/utils/concepts.h"

namespace call_center::core::containers {

/**
 * @brief Реализация очереди, поддерживающая операцию @link Contains @endlink за O(1).
 */
template <NoThrowCopyConstructor T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
class Queue {
 public:
  explicit Queue(size_t capacity = SIZE_MAX);
  Queue(const Queue &other) = delete;
  Queue &operator=(const Queue &other) = delete;

  bool Push(const T &t);
  std::optional<T> Pop();
  [[nodiscard]] bool IsEmpty() const;
  [[nodiscard]] bool IsFull() const;
  /**
   * @brief Проверка наличия заданного элемента в очереди за O(1).
   */
  bool Contains(const T &t);
  [[nodiscard]] std::optional<T> Peek() const;
  void SetCapacity(size_t capacity);

 private:
  std::queue<T> queue_;
  std::unordered_multiset<T, Hash, Equal> set_;
  size_t capacity_;
};

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
void Queue<T, Hash, Equal>::SetCapacity(const size_t capacity) {
  capacity_ = capacity;
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
std::optional<T> Queue<T, Hash, Equal>::Peek() const {
  if (queue_.empty())
    return std::nullopt;

  return queue_.front();
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
bool Queue<T, Hash, Equal>::IsFull() const {
  return queue_.size() >= capacity_;
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
bool Queue<T, Hash, Equal>::Contains(const T &t) {
  return set_.contains(t);
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
std::optional<T> Queue<T, Hash, Equal>::Pop() {
  if (queue_.empty())
    return std::nullopt;

  auto item = queue_.front();
  queue_.pop();
  set_.erase(set_.find(item));
  return item;
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
Queue<T, Hash, Equal>::Queue(const size_t capacity) : capacity_(capacity) {
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
bool Queue<T, Hash, Equal>::Push(const T &t) {
  if (queue_.size() == capacity_)
    return false;

  queue_.emplace(t);
  set_.emplace(t);
  return true;
}

template <NoThrowCopyConstructor T, typename Hash, typename Equal>
bool Queue<T, Hash, Equal>::IsEmpty() const {
  return queue_.empty();
}

}  // namespace call_center::core::containers

#endif  // CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_QUEUE_H_
