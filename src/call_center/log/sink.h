#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_SINK_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_SINK_H_

#include <boost/log/sinks.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/move/unique_ptr.hpp>

#include <string>
#include <optional>
#include <ostream>

#include "severity_level.h"

namespace call_center::log {

class Sink {
 public:
  using Formatter = std::function<void(const boost::log::record_view &rec, boost::log::formatting_ostream &out)>;

  explicit Sink(SeverityLevel level);
  Sink(const std::string &file_name, SeverityLevel level, size_t max_size);
  Sink(const std::string &file_name, SeverityLevel level, size_t max_size, const Formatter &formatter);
  Sink(SeverityLevel level, const Formatter &formatter);
  Sink(boost::shared_ptr<std::ostream> ostream,
       SeverityLevel level,
       const Formatter &formatter,
       size_t max_size = SIZE_MAX);
  Sink(Sink &other) = delete;
  Sink(Sink &&other) = default;
  Sink &operator=(Sink &other) = delete;
  Sink &operator=(Sink &&other) = default;

  [[nodiscard]] const boost::uuids::uuid &Id() const;

 private:
  using SinkImpl = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

  boost::shared_ptr<SinkImpl> sink_impl_;
  boost::shared_ptr<std::ostream> stream_;
  boost::uuids::uuid id_ = boost::uuids::random_generator_mt19937()();
  SeverityLevel level_;
  size_t max_size_;

  static void DefaultFormatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out);
};

}

#endif //CALL_CENTER_SRC_CALL_CENTER_LOG_SINK_H_
