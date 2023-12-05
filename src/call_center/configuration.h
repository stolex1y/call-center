#ifndef CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
#define CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_

#include <any>
#include <boost/json.hpp>
#include <optional>
#include <shared_mutex>
#include <string>

#include "core/containers/concurrent_hash_map.h"
#include "log/logger.h"
#include "log/logger_provider.h"

namespace call_center {
using namespace core::utils::concepts;

/**
 * @brief Класс, выполняющий чтение конфигурации из файла.
 */
class Configuration : public std::enable_shared_from_this<Configuration> {
 public:
  static std::shared_ptr<Configuration> Create(
      const log::LoggerProvider &logger_provider, std::string file_name = kDefaultFileName
  );

  static constexpr auto kDefaultFileName = "config.json";
  /// Ключ в конфигурации, соответствующий значению свойства кеширования других значений из
  /// конфигурации.
  static constexpr auto kCachingKey = "configuration_is_caching";

  Configuration(Configuration &other) = delete;
  Configuration &operator=(Configuration &other) = delete;

  /**
   * @brief Получить значение свойства по ключу.
   * @return std::nullopt - если ключе не был найден.
   */
  template <typename T>
  std::optional<T> GetProperty(const std::string &key);

  /**
   * @brief Получить значение свойства по ключу либо переданное значение по умолчанию.
   */
  template <typename T>
  T GetProperty(const std::string &key, const T &default_value);

  /**
   * @brief Получить числовое значение из конфигурации либо переданное значение по умолчанию, если
   * значение в конфигурации не лежит в заданных пределах либо такой ключ не найден. @param key ключ
   * @param default_value значение по умолчанию
   * @param min минимально допустимое значение
   * @param max максимально допустимое значение
   */
  template <Arithmetic T>
  T GetNumber(
      const std::string &key,
      T default_value,
      T min = std::numeric_limits<T>::min(),
      T max = std::numeric_limits<T>::max()
  );

  /**
   * @brief Имя файла с конфигурацией.
   */
  const std::string &GetFileName() const;

  /**
   * @brief Повторно прочитать конфигураию из файла.
   */
  void UpdateConfiguration();

 private:
  static constexpr bool kDefaultCaching_ = true;

  std::atomic_bool caching_ = kDefaultCaching_;
  std::unique_ptr<log::Logger> logger_;
  boost::json::object config_json_;
  mutable std::shared_mutex config_mutex_;
  core::containers::ConcurrentHashMap<std::string, std::any> cache_values_;
  const std::string file_name_;

  explicit Configuration(
      const log::LoggerProvider &logger_provider, std::string file_name = kDefaultFileName
  );

  /**
   * @brief Обновить значение свойства кеширования из конфигурации.
   */
  void UpdateCaching();

  /**
   * @brief Базовый метод для остальных, непосредственно читающий значение из конфигурации.
   */
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
