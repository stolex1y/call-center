#include "logger_provider.h"

namespace call_center::log {

LoggerProvider::LoggerProvider(std::unique_ptr<Sink> sink)
    : sink_(std::move(sink)) {}

std::unique_ptr<Logger> LoggerProvider::Get(std::string tag) const {
  return std::make_unique<Logger>(std::move(tag), *sink_);
}

}
