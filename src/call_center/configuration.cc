#include "configuration.h"

#include <fstream>

namespace call_center {

std::shared_ptr<Configuration> Configuration::Create(
    const std::shared_ptr<const log::LoggerProvider> &logger_provider
) {
  return std::shared_ptr<Configuration>(new Configuration(logger_provider));
}

Configuration::Configuration(const std::shared_ptr<const log::LoggerProvider> &logger_provider)
    : logger_(logger_provider->Get("Configuration")) {
  UpdateConfiguration();
}

void Configuration::UpdateConfiguration() {
  std::lock_guard lock(config_mutex_);

  std::ifstream config_file(kFileName_);
  if (!config_file) {
    logger_->Warning() << "Couldn't open configuration file: " << kFileName_;
    return;
  }

  boost::json::error_code error;
  auto &&json_value = boost::json::parse(config_file, error);
  if (error || !json_value.is_object()) {
    config_json_ = {};
    logger_->Error() << "Invalid json format in configuration file: " << error.what();
    return;
  }
  config_json_ = std::move(json_value.as_object());

  cache_values_.Clear();
  UpdateCaching();
}

void Configuration::UpdateCaching() {
  caching_ = ReadProperty<bool>(kCachingKey_).value_or(caching_);
}

}  // namespace call_center
