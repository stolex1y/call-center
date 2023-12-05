#ifndef CALL_CENTER_LOGGER_H
#define CALL_CENTER_LOGGER_H

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <optional>
#include <string>

#include "severity_level.h"
#include "sink.h"

/// Логирование.
namespace call_center::log {

/**
 * @brief Класс-обертка над boost::log::sources::serverity_logger_mt.
 */
class Logger : boost::log::sources::severity_logger_mt<SeverityLevel> {
 public:
  /**
   * @brief Поток вывода логов.
   */
  class Ostream {
   public:
    explicit Ostream(Logger &log, SeverityLevel level);
    Ostream(const Ostream &other) = delete;
    Ostream &operator=(const Ostream &other) = delete;
    /**
     * @brief При уничтожении отправляет запись в @link Logger @endlink.
     */
    ~Ostream();

    template <typename T>
    Ostream &operator<<(const T &t) {
      ostream_impl_ << t;
      return *this;
    }

   private:
    using OstreamImpl = boost::log::record_ostream;

    Logger &logger_;
    boost::log::record record_;
    OstreamImpl ostream_impl_;
  };

  Logger(std::string tag, std::shared_ptr<Sink> sink);
  Logger(const Logger &other) = delete;
  Logger &operator=(const Logger &other) = delete;

  /**
   * @brief Открыть поток вывода логов уровня @link SeverityLevel::kTrace @endlink.
   */
  Ostream Trace();
  /**
   * @brief Открыть поток вывода логов уровня @link SeverityLevel::kDebug @endlink.
   */
  Ostream Debug();
  /**
   * @brief Открыть поток вывода логов уровня @link SeverityLevel::kInfo @endlink.
   */
  Ostream Info();
  /**
   * @brief Открыть поток вывода логов уровня @link SeverityLevel::kWarning @endlink.
   */
  Ostream Warning();
  /**
   * @brief Открыть поток вывода логов уровня @link SeverityLevel::kError @endlink.
   */
  Ostream Error();
  /**
   * @brief Открыть поток вывода логов уровня @link SeverityLevel::kFatal @endlink.
   */
  Ostream Fatal();

 private:
  std::shared_ptr<Sink> sink_;
};

}  // namespace call_center::log

#endif  // CALL_CENTER_LOGGER_H
