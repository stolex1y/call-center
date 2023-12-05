#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_LOGGER_PROVIDER_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_LOGGER_PROVIDER_H_

#include <memory>

#include "logger.h"
#include "sink.h"

namespace call_center::log {

/**
 * @brief Класс для предоставления @link Logger логера@endlink, связанного с определенным @link Sink
 * приёмником@endlink.
 */
class LoggerProvider {
 public:
  explicit LoggerProvider(std::shared_ptr<Sink> sink);

  /**
   * @brief Создать логер с заданным тегом.
   */
  [[nodiscard]] std::unique_ptr<Logger> Get(std::string tag = "") const;

 private:
  std::shared_ptr<Sink> sink_;
};

}  // namespace call_center::log

#endif  // CALL_CENTER_SRC_CALL_CENTER_LOG_LOGGER_PROVIDER_H_
