#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_SINK_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_SINK_H_

#include <boost/log/sinks.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <ostream>
#include <string>

#include "severity_level.h"

namespace call_center::log {

/**
 * @brief Класс приёмника логов, связанный с потоком вывода.
 */
class Sink {
 public:
  /**
   * @brief Функция форматирования логов.
   */
  using Formatter =
      std::function<void(const boost::log::record_view &rec, boost::log::formatting_ostream &out)>;

  /**
   * @brief Использует стандартный поток вывода для записи логов.
   * @param level уровень логирования
   */
  explicit Sink(SeverityLevel level);
  /**
   * @brief Использует указанный файл для записи логов.
   * @param file_name название файла
   * @param level уровень логирования
   * @param max_size максимальный размер файла
   */
  Sink(const std::string &file_name, SeverityLevel level, size_t max_size);
  /**
   * @brief Использует указанный файл для записи логов.
   * @param file_name название файла
   * @param level уровень логирования
   * @param max_size максимальный размер файла
   * @param formatter функция для форматирования логов
   */
  Sink(
      const std::string &file_name, SeverityLevel level, size_t max_size, const Formatter &formatter
  );
  /**
   * @brief Использует стандартный поток вывода для записи логов.
   * @param level уровень логирования
   * @param formatter функция для форматирования логов
   */
  Sink(SeverityLevel level, const Formatter &formatter);
  /**
   * @brief Использует указанный поток вывода для записи логов.
   * @param ostream поток для записи логов
   * @param level уровень логирования
   * @param formatter функция для форматирования логов
   * @param max_size максимальный размер файла
   */
  Sink(
      boost::shared_ptr<std::ostream> ostream,
      SeverityLevel level,
      const Formatter &formatter,
      size_t max_size = SIZE_MAX
  );
  ~Sink();
  Sink(Sink &other) = delete;
  Sink(Sink &&other) = default;
  Sink &operator=(Sink &other) = delete;
  Sink &operator=(Sink &&other) = default;

  /**
   * @brief Идентификатор приемника логов для связи с @link Logger логерами@endlink.
   */
  [[nodiscard]] const boost::uuids::uuid &Id() const;

 private:
  using SinkImpl = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

  boost::shared_ptr<SinkImpl> sink_impl_;
  boost::shared_ptr<std::ostream> stream_;
  boost::uuids::uuid id_ = boost::uuids::random_generator_mt19937()();
  SeverityLevel level_;
  size_t max_size_;

  /**
   * @brief Форматирование логов по умолчанию.
   * @param rec запись лога
   * @param out поток форматированного вывода
   */
  static void DefaultFormatter(
      const boost::log::record_view &rec, boost::log::formatting_ostream &out
  );
};

}  // namespace call_center::log

#endif  // CALL_CENTER_SRC_CALL_CENTER_LOG_SINK_H_
