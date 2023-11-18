#include "logger.h"

#include <boost/log/attributes.hpp>

#include "attrs.h"

namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace boost_attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace call_center::log {

Logger::Logger(std::string tag, const Sink &sink) {
  add_attribute(attrs::tag_attr_type::get_name(), boost_attrs::constant<std::string>(std::move(tag)));
  add_attribute(attrs::channel_type::get_name(), boost_attrs::constant<boost::uuids::uuid>(sink.Id()));
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

Logger::Ostream::Ostream(Logger &logger, SeverityLevel level)
    : logger_(logger),
      record_(logger.open_record(keywords::severity = level)) {
  if (record_) {
    ostream_impl_.attach_record(record_);
  }
}

Logger::Ostream::~Ostream() {
  if (record_) {
    ostream_impl_.flush();
    logger_.push_record(std::move(record_));
  }
}

}
