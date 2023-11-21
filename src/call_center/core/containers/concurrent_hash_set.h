#ifndef CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_HASH_SET_H_
#define CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_HASH_SET_H_

#include <mutex>
#include <optional>
#include <unordered_set>

#include "core/utils/concepts.h"

namespace call_center::core::containers {

using namespace core::utils::concepts;

template <NoThrowMoveConstructor T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
class ConcurrentHashSet {
 public:
  ConcurrentHashSet(const ConcurrentHashSet<T, Hash, Equal> &other) = delete;
  ConcurrentHashSet &operator=(const ConcurrentHashSet<T, Hash, Equal> &other) = delete;

  bool Insert(T t);
  bool Erase(const T &t);
  bool Empty() const;

 private:
  mutable std::mutex mutex_;
  std::unordered_set<T, Hash, Equal> set_;
};

template <NoThrowMoveConstructor T, typename Hash, typename Equal>
bool ConcurrentHashSet<T, Hash, Equal>::Empty() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return set_.empty();
}

template <NoThrowMoveConstructor T, typename Hash, typename Equal>
bool ConcurrentHashSet<T, Hash, Equal>::Erase(const T &t) {
  std::lock_guard<std::mutex> lock(mutex_);
  return set_.erase(t) > 0;
}

template <NoThrowMoveConstructor T, typename Hash, typename Equal>
bool ConcurrentHashSet<T, Hash, Equal>::Insert(T t) {
  std::lock_guard<std::mutex> lock(mutex_);
  const auto [it, added] = set_.emplace(std::move(t));
  return added;
}

}  // namespace call_center::core::containers

#endif  // CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_HASH_SET_H_
