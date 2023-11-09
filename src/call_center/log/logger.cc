#include "logger.h"

#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <thread>

namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace call_center::log {

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", uint)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", SeverityLevel)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread, "ThreadId", boost::log::thread_id)

static std::string InitLoggersWithFile(const std::string &file_name) {
  Logger::SetFileName(file_name);
  return file_name;
}

SeverityLevel Logger::level_ = SeverityLevel::kInfo;
int Logger::max_size_ = 10;
std::string Logger::file_name_ = InitLoggersWithFile("logs");

Logger::Logger(std::string tag) : tag_(std::move(tag)) {
  add_attribute("Tag", attrs::constant<std::string>(tag_));
}

Logger::Ostream Logger::Trace() {
  return Logger::Ostream(*this, SeverityLevel::kTrace);
}

Logger::Ostream Logger::Debug() {
  return Logger::Ostream(*this, SeverityLevel::kDebug);
}

Logger::Ostream Logger::Info() {
  return Logger::Ostream(*this, SeverityLevel::kInfo);
}

Logger::Ostream Logger::Warning() {
  return Logger::Ostream(*this, SeverityLevel::kWarning);
}

Logger::Ostream Logger::Error() {
  return Logger::Ostream(*this, SeverityLevel::kError);
}

Logger::Ostream Logger::Fatal() {
  return Logger::Ostream(*this, SeverityLevel::kFatal);
}

void Logger::Formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out) {
  out << "[" << rec[line_id] << "] ";
  out << "[" << to_simple_string(rec[timestamp].get()) << "] ";
  if (!rec[thread].empty()) {
    out << "[" << rec[thread] << "] ";
  }
  out << "[" << rec[severity] << "] ";
  out << "[" << rec[tag_attr] << "] ";
  out << rec[expr::message];
}

void Logger::SetFileName(const std::string &file_name) {
  if (file_name == file_name_)
    return;
  file_name_ = file_name;
  UpdateLogger();
}

void Logger::SetLevel(SeverityLevel level) {
  if (level_ == level)
    return;
  level_ = level;
  UpdateLogger();
}

void Logger::SetMaxSize(int max_size) {
  if (max_size_ == max_size)
    return;
  max_size_ = max_size;
  UpdateLogger();
}

void Logger::UpdateLogger() {
  auto core = boost::log::core::get();
  core->remove_all_sinks();
  boost::log::add_file_log(keywords::file_name = file_name_,
                           keywords::max_size = max_size_,
                           keywords::format = Logger::Formatter,
                           keywords::open_mode = std::ios_base::app);
  boost::log::add_common_attributes();
  core->set_filter(severity >= level_);
}

Logger::Ostream::Ostream(Logger &logger, SeverityLevel level)
    : logger_(logger),
      record_(logger.open_record(keywords::severity = level)) {
  if (record_) {
    attach_record(record_);
  }
}

Logger::Ostream::~Ostream() {
  if (record_) {
    flush();
    logger_.push_record(std::move(record_));
  }
}

std::ostream &operator<<(std::ostream &out, SeverityLevel level) {
  switch (level) {
    case SeverityLevel::kTrace: {
      out << "TRACE";
      break;
    }
    case SeverityLevel::kDebug: {
      out << "DEBUG";
      break;
    }
    case SeverityLevel::kInfo: {
      out << "INFO";
      break;
    }
    case SeverityLevel::kWarning: {
      out << "WARNING";
      break;
    }
    case SeverityLevel::kError: {
      out << "ERROR";
      break;
    }
    case SeverityLevel::kFatal: {
      out << "FATAL";
      break;
    }
  }
  return out;
}

}