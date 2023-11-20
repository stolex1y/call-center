#include "configuration.h"

namespace call_center {

Configuration::Configuration(
    const std::shared_ptr<const log::LoggerProvider> &logger_provider
)
    : logger_(logger_provider->Get("Configuration")) {
}

}  // namespace call_center
