#include "logger.h"

#include <boost/log/attributes.hpp>

#include "attrs.h"

namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace boost_attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace call_center::log {

Logger::Logger(std::string tag, std::shared_ptr<Sink> sink) : sink_(std::move(sink)) {
  add_attribute(attrs::tag_attr_type::get_name(), boost_attrs::constant(std::move(tag)));
  add_attribute(attrs::channel_type::get_name(), boost_attrs::constant(sink_->Id()));
  add_attribute(attrs::line_id_type::get_name(), boost::log::attributes::counter<unsigned int>(1));
  add_attribute(attrs::timestamp_type::get_name(), boost::log::attributes::local_clock());
  add_attribute(attrs::thread_type::get_name(), boost::log::attributes::current_thread_id());
}

Logger::Ostream Logger::Trace() {
  return Ostream(*this, SeverityLevel::kTrace);
}

Logger::Ostream Logger::Debug() {
  return Ostream(*this, SeverityLevel::kDebug);
}

Logger::Ostream Logger::Info() {
  return Ostream(*this, SeverityLevel::kInfo);
}

Logger::Ostream Logger::Warning() {
  return Ostream(*this, SeverityLevel::kWarning);
}

Logger::Ostream Logger::Error() {
  return Ostream(*this, SeverityLevel::kError);
}

Logger::Ostream Logger::Fatal() {
  return Ostream(*this, SeverityLevel::kFatal);
}

Logger::Ostream::Ostream(Logger &logger, SeverityLevel level)
    : logger_(logger), record_(logger.open_record(keywords::severity = level)) {
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

}  // namespace call_center::log
