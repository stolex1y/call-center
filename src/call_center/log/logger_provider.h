#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_LOGGER_PROVIDER_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_LOGGER_PROVIDER_H_

#include <memory>

#include "logger.h"
#include "sink.h"

namespace call_center::log {

class LoggerProvider {
 public:
  explicit LoggerProvider(std::unique_ptr<Sink> sink);

  [[nodiscard]] std::unique_ptr<Logger> Get(std::string tag = "") const;

 private:
  std::unique_ptr<Sink> sink_;
};

}

#endif //CALL_CENTER_SRC_CALL_CENTER_LOG_LOGGER_PROVIDER_H_
