#ifndef CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
#define CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_

#include <any>
#include <boost/core/ignore_unused.hpp>
#include <boost/json.hpp>
#include <optional>
#include <shared_mutex>
#include <string>

#include "core/containers/concurrent_hash_map.h"
#include "log/logger.h"
#include "log/logger_provider.h"

namespace call_center {

class Configuration : public std::enable_shared_from_this<Configuration> {
 public:
  static std::shared_ptr<Configuration> Create(
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );

  static constexpr const auto kFileName_ = "config.json";
  static constexpr const auto kCachingKey_ = "configuration_is_caching";

  Configuration(Configuration &other) = delete;
  Configuration &operator=(Configuration &other) = delete;

  template <typename T>
  std::optional<T> GetProperty(const std::string &key);

  template <typename T>
  T GetProperty(const std::string &key, const T &default_value);

  template <Arithmetic T>
  T GetNumber(
      const std::string &key,
      T default_value,
      T min = std::numeric_limits<T>::min(),
      T max = std::numeric_limits<T>::max()
  );

  void UpdateConfiguration();

 private:
  static constexpr const bool kDefaultCaching_ = true;

  std::atomic_bool caching_ = kDefaultCaching_;
  std::unique_ptr<log::Logger> logger_;
  boost::json::object config_json_;
  mutable std::shared_mutex config_mutex_;
  core::containers::ConcurrentHashMap<std::string, std::any> cache_values_;

  explicit Configuration(const std::shared_ptr<const log::LoggerProvider> &logger_provider);

  void UpdateCaching();

  template <typename T>
  std::optional<T> ReadProperty(const std::string &key);
};

template <typename T>
std::optional<T> Configuration::GetProperty(const std::string &key) {
  if (caching_) {
    const auto value = cache_values_.Get(key);
    if (value) {
      logger_->Trace() << "Get value by key '" << key << "' from cache";
      return any_cast<T>(*value);
    }
  } else {
    UpdateConfiguration();
  }

  std::shared_lock lock(config_mutex_);
  const auto &&value = ReadProperty<T>(key);
  if (caching_ && value) {
    cache_values_.Set(key, *value);
  }
  return value;
}

template <typename T>
T Configuration::GetProperty(const std::string &key, const T &default_value) {
  return GetProperty<T>(key).value_or(default_value);
}

template <typename T>
std::optional<T> Configuration::ReadProperty(const std::string &key) {
  if (!config_json_.contains(key)) {
    return std::nullopt;
  }

  auto result = try_value_to<T>(config_json_.at(key));
  if (!result.has_value()) {
    logger_->Error() << "Couldn't parse value as '" << typeid(T).name() << "' by key '" << key
                     << "'. " << result.error().what();
    return std::nullopt;
  } else {
    return result.value();
  }
}

template <Arithmetic T>
T Configuration::GetNumber(const std::string &key, T default_value, T min, T max) {
  const auto value = GetProperty<T>(key, default_value);
  if (value < min || value > max) {
    logger_->Warning() << "Value by key '" << key << "' isn't in range [" << min << ", " << max
                       << "]";
  }
  return std::max(min, std::min(max, value));
}

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
