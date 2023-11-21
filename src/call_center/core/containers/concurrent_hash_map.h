#ifndef CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_HASH_SET_H_
#define CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_HASH_SET_H_

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "core/utils/concepts.h"

namespace call_center::core::containers {

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash = std::hash<K>,
    typename Equal = std::equal_to<K>>
class ConcurrentHashMap {
 public:
  ConcurrentHashMap() = default;
  ConcurrentHashMap(const ConcurrentHashMap<V, Hash, Equal> &other) = delete;
  ConcurrentHashMap &operator=(const ConcurrentHashMap<V, Hash, Equal> &other
  ) = delete;

  void Set(K key, V value);
  bool Erase(const K &key);
  bool Empty() const;
  std::optional<V> Get(const K &key) const;
  void Clear();
  bool Contains(const K &key);

 private:
  mutable std::shared_mutex mutex_;
  std::unordered_map<K, V, Hash, Equal> map_{};
};

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash,
    typename Equal>
bool ConcurrentHashMap<K, V, Hash, Equal>::Contains(const K &key) {
  std::shared_lock lock(mutex_);
  return map_.contains(key);
}

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash,
    typename Equal>
void ConcurrentHashMap<K, V, Hash, Equal>::Clear() {
  std::lock_guard lock(mutex_);
  map_.clear();
}

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash,
    typename Equal>
std::optional<V> ConcurrentHashMap<K, V, Hash, Equal>::Get(const K &key) const {
  std::shared_lock lock(mutex_);
  if (!map_.contains(key))
    return std::nullopt;
  return *map_.find(key);
}

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash,
    typename Equal>
bool ConcurrentHashMap<K, V, Hash, Equal>::Empty() const {
  std::shared_lock lock(mutex_);
  return map_.empty();
}

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash,
    typename Equal>
bool ConcurrentHashMap<K, V, Hash, Equal>::Erase(const K &key) {
  std::lock_guard lock(mutex_);
  return map_.erase(key) > 0;
}

template <
    NoThrowMoveConstructor K,
    NoThrowMoveConstructor V,
    typename Hash,
    typename Equal>
void ConcurrentHashMap<K, V, Hash, Equal>::Set(K key, V value) {
  std::lock_guard lock(mutex_);
  map_[std::move(key)] = std::move(value);
}

}  // namespace call_center::core::containers

#endif  // CALL_CENTER_SRC_CALL_CENTER_UTILS_CONCURRENT_HASH_SET_H_
