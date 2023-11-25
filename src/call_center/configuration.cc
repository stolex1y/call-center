#include "configuration.h"

#include <fstream>

namespace call_center {

std::shared_ptr<Configuration> Configuration::Create(
    const log::LoggerProvider &logger_provider, std::string file_name
) {
  return std::shared_ptr<Configuration>(new Configuration(logger_provider, std::move(file_name)));
}

Configuration::Configuration(const log::LoggerProvider &logger_provider, std::string file_name)
    : logger_(logger_provider.Get("Configuration")), file_name_(std::move(file_name)) {
  UpdateConfiguration();
}

void Configuration::UpdateConfiguration() {
  std::lock_guard lock(config_mutex_);

  std::ifstream config_file(file_name_);
  if (!config_file) {
    logger_->Warning() << "Couldn't open configuration file: " << file_name_;
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

const std::string &Configuration::GetFileName() const {
  return file_name_;
}

}  // namespace call_center
