#include "sink.h"

#include <iostream>
#include <fstream>

#include <boost/log/core.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "attrs.h"

namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace boost_attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace call_center::log {

Sink::Sink(SeverityLevel level, size_t max_size)
    : Sink(level, max_size, Sink::DefaultFormatter) {
}

Sink::Sink(const std::string &file_name, SeverityLevel level, size_t max_size)
    : Sink(file_name, level, max_size, Sink::DefaultFormatter) {
}

Sink::Sink(SeverityLevel level, size_t max_size, const Formatter &formatter)
    : Sink(boost::shared_ptr<std::ostream>(&std::cout, [](std::ostream *) {}),
           level, max_size, formatter) {
}

Sink::Sink(const std::string &file_name, SeverityLevel level,
           size_t max_size, const Formatter &formatter)
    : Sink(boost::make_shared<std::ofstream>(file_name),
           level, max_size, formatter) {
}

Sink::Sink(boost::shared_ptr<std::ostream> ostream, SeverityLevel level,
           size_t max_size, const Formatter &formatter)
    : sink_impl_(new SinkImpl(keywords::max_size = max_size * 1024 * 1024)), stream_(std::move(ostream)),
      level_(level), max_size_(max_size) {
  boost::log::add_common_attributes();

  sink_impl_->set_formatter(formatter);
  sink_impl_->set_filter(attrs::severity >= level_ && attrs::channel == id_);
  sink_impl_->locked_backend()->add_stream(stream_);
  sink_impl_->locked_backend()->auto_flush(true);

  boost::log::core::get()->add_sink(sink_impl_);
}

const boost::uuids::uuid &Sink::Id() const {
  return id_;
}

void Sink::DefaultFormatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out) {
  out << "[" << to_simple_string(rec[attrs::timestamp].get()) << "] ";
  if (!rec[attrs::thread].empty()) {
    out << "tid[" << rec[attrs::thread] << "] ";
  }
  out << "[" << rec[attrs::severity] << "] ";
  out << "[" << rec[attrs::tag_attr] << "] ";
  out << rec[expr::message];
}

} // log
