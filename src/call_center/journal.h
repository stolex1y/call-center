#ifndef CALL_CENTER_SRC_CALL_CENTER_JOURNAL_H_
#define CALL_CENTER_SRC_CALL_CENTER_JOURNAL_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/sinks.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <optional>
#include <string>

#include "call_detailed_record.h"
#include "configuration/configuration.h"
#include "log/logger.h"
#include "log/severity_level.h"
#include "log/sink.h"

namespace call_center {

/**
 * @brief Журнал для записи информации о полученных вызовах.
 *
 * Записывает информацию в файл.
 * Основан на @link log::Logger логере@endlink и @link log::Sink приемнике@endlink.
 */
class Journal {
 public:
  explicit Journal(std::shared_ptr<config::Configuration> configuration);
  Journal(const Journal &other) = delete;
  Journal &operator=(const Journal &other) = delete;

  /**
   * @brief Добавить вызов в журнал.
   */
  void AddRecord(const CallDetailedRecord &cdr);

 private:
  static constexpr auto kFileNameKey_ = "journal_file_name";
  static constexpr auto kDefaultFileName_ = "journal.csv";

  /// Ключ в конфигурации, соответствующий значению максимального размера файла в Мб.
  static constexpr auto kMaxSizeKey_ = "journal_max_size";
  static constexpr size_t kDefaultMaxSize_ = SIZE_MAX;

  std::shared_ptr<config::Configuration> configuration_;
  std::string file_name_ = kDefaultFileName_;
  size_t max_size_ = kDefaultMaxSize_;
  std::shared_ptr<log::Sink> sink_;
  std::unique_ptr<log::Logger> logger_;

  /**
   * @brief Задает формат логов (записей вызовов).
   */
  static void Formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out);
  /**
   * @brief Преобразовать вызов в строку.
   */
  static std::string FormatCallDetailedRecord(const CallDetailedRecord &cdr);
  /**
   * @brief Преобразовать временную точку в строку формата: yyyy-mm-dd hh24:mm:ss.fff.
   */
  static std::string FormatTimePoint(const CallDetailedRecord::TimePoint &time_point);
  /**
   * @brief Преобразовать временную точку в строку формата: yyyy-mm-dd hh24:mm:ss.fff.
   * @return "" - если аргумент равен std::nullopt.
   */
  static std::string FormatTimePoint(const std::optional<CallDetailedRecord::TimePoint> &time_point
  );
  /**
   * @brief Преобразовать boost::uuids::uuid в строку.
   */
  static std::string FormatUuid(const boost::uuids::uuid &uuid);
  /**
   * @brief Преобразовать boost::uuids::uuid в строку.
   * @return "" - если аргумент равен std::nullopt.
   */
  static std::string FormatUuid(const std::optional<boost::uuids::uuid> &uuid);
  /**
   * @brief Преобразовать временной промежуток в строку формата: hh24:mm:ss.fff.
   */
  static std::string FormatDuration(const CallDetailedRecord::Duration &duration);
  /**
   * @brief Преобразовать временной промежуток в строку формата: hh24:mm:ss.fff.
   * @return "" - если аргумент равен std::nullopt.
   */
  static std::string FormatDuration(const std::optional<CallDetailedRecord::Duration> &duration);

  /**
   * @brief Прочитать название файла журнала из конфигурации.
   */
  [[nodiscard]] std::string ReadFileName() const;
  /**
   * @brief Прочитать максимальный размер журнала из конфигурации.
   */
  [[nodiscard]] size_t ReadMaxSize() const;
  /**
   * @brief Обновить приемник.
   *
   * Обновляет приемник, если значения @link max_size_ @endlink или @link file_name_ @endlink
   * изменились.
   */
  void UpdateSink();
  /**
   * @brief Создать приемник.
   */
  std::shared_ptr<log::Sink> MakeSink();
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_JOURNAL_H_
