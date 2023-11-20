#ifndef CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
#define CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_

#include <boost/core/ignore_unused.hpp>
#include <optional>
#include <string>

#include "log/logger.h"
#include "log/logger_provider.h"

namespace call_center {

class Configuration {
 public:
  explicit Configuration(
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );

  template <typename T>
  std::optional<T> GetProperty(const std::string &key) const;

 private:
  std::unique_ptr<log::Logger> logger_;
};

template <typename T>
std::optional<T> Configuration::GetProperty(const std::string &key) const {
  boost::ignore_unused(key);
  return std::nullopt;
}

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_H_
