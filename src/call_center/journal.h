#ifndef CALL_CENTER_SRC_CALL_CENTER_JOURNAL_H_
#define CALL_CENTER_SRC_CALL_CENTER_JOURNAL_H_

#include <boost/log/sinks.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_facet.hpp>

#include <string>
#include <optional>
#include <ostream>

#include "log/severity_level.h"
#include "log/sink.h"
#include "log/logger.h"
#include "call_detailed_record.h"
#include "configuration.h"

namespace call_center {

class Journal {
 public:
  explicit Journal(const Configuration &configuration);

  void AddRecord(const CallDetailedRecord &cdr);

 private:
  static constexpr const auto kFileNameKey = "journal_file_name";
  static constexpr const auto kDefaultFileName = "journal.csv";

  static constexpr const auto kMaxSizeKey = "journal_max_size";
  static constexpr const size_t kDefaultMaxSize = 100;

  const Configuration &configuration_;
  std::string file_name_;
  log::Sink sink_;
  log::Logger logger_;
  size_t max_size_;

  static void Formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out);
  static std::string FormatCallDetailedRecord(const CallDetailedRecord &cdr);
  static std::string FormatTimePoint(const CallDetailedRecord::TimePoint &time_point);
  static std::string FormatTimePoint(const std::optional<CallDetailedRecord::TimePoint> &time_point);
  static std::string FormatUuid(const boost::uuids::uuid &uuid);
  static std::string FormatUuid(const std::optional<boost::uuids::uuid> &uuid);
  static std::string FormatDuration(const CallDetailedRecord::Duration &duration);
  static std::string FormatDuration(const std::optional<CallDetailedRecord::Duration> &duration);

  [[nodiscard]] std::string ReadFileName() const;
  [[nodiscard]] size_t ReadMaxSize() const;
  void UpdateSink();
  log::Sink MakeSink();
};

}

#endif //CALL_CENTER_SRC_CALL_CENTER_JOURNAL_H_
