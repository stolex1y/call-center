#include "journal.h"

#include <boost/uuid/uuid_io.hpp>
#include <format>
#include <fstream>

#include "core/utils/date_time.h"

namespace call_center {

namespace expr = boost::log::expressions;
namespace date_time_utils = core::utils::date_time;

Journal::Journal(std::shared_ptr<Configuration> configuration)
    : configuration_(std::move(configuration)) {
  file_name_ = ReadFileName();
  max_size_ = ReadMaxSize();
  sink_ = MakeSink();
  logger_ = std::make_unique<log::Logger>("Journal", *sink_);
}

void Journal::Formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out) {
  out << rec[expr::message];
}

std::string Journal::ReadFileName() const {
  return configuration_->GetProperty(kFileNameKey_, file_name_);
}

std::string Journal::FormatCallDetailedRecord(const CallDetailedRecord &cdr) {
  return std::vformat(
      "{};{};{};{};{};{};{};{}",
      std::make_format_args(
          FormatTimePoint(cdr.GetReceiptTime()),
          FormatUuid(cdr.GetId()),
          cdr.GetCallerPhoneNumber(),
          FormatTimePoint(cdr.GetEndProcessingTime()),
          to_string(cdr.GetStatus()),
          FormatTimePoint(cdr.GetStartProcessingTime()),
          FormatUuid(cdr.GetOperatorId()),
          FormatDuration(cdr.GetProcessingDuration())
      )
  );
}

void Journal::AddRecord(const CallDetailedRecord &cdr) {
  UpdateSink();
  auto result = FormatCallDetailedRecord(cdr);
  logger_->Info() << result;
}

std::string Journal::FormatTimePoint(const std::optional<CallDetailedRecord::TimePoint> &time_point
) {
  if (time_point)
    return FormatTimePoint(time_point.value());
  else
    return "";
}

std::string Journal::FormatTimePoint(const CallDetailedRecord::TimePoint &time_point) {
  return std::vformat("{0:%F} {0:%T}", std::make_format_args(time_point));
}

std::string Journal::FormatDuration(const CallDetailedRecord::Duration &duration) {
  return std::vformat("{0:%T}", std::make_format_args(duration));
}

std::string Journal::FormatDuration(const std::optional<CallDetailedRecord::Duration> &duration) {
  if (duration)
    return FormatDuration(*duration);
  else
    return "";
}

std::string Journal::FormatUuid(const boost::uuids::uuid &uuid) {
  return boost::uuids::to_string(uuid);
}

std::string Journal::FormatUuid(const std::optional<boost::uuids::uuid> &uuid) {
  if (uuid)
    return boost::uuids::to_string(*uuid);
  else
    return "";
}

void Journal::UpdateSink() {
  auto new_file_name = ReadFileName();
  auto new_max_size = ReadMaxSize();
  if (new_file_name != file_name_ || new_max_size != max_size_) {
    file_name_ = std::move(new_file_name);
    max_size_ = new_max_size;
    sink_ = MakeSink();
    logger_ = std::make_unique<log::Logger>("Journal", *sink_);
  }
}

std::unique_ptr<log::Sink> Journal::MakeSink() {
  return std::make_unique<log::Sink>(
      boost::make_shared<std::ofstream>(file_name_, std::ios_base::app),
      log::SeverityLevel::kTrace,
      Journal::Formatter,
      max_size_
  );
}

size_t Journal::ReadMaxSize() const {
  return configuration_->GetNumber<size_t>(kMaxSizeKey_, max_size_, 1);
}

}  // namespace call_center
