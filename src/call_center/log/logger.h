#ifndef CALL_CENTER_LOGGER_H
#define CALL_CENTER_LOGGER_H

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <optional>
#include <string>

#include "severity_level.h"
#include "sink.h"

namespace call_center::log {

class Logger : private boost::log::sources::severity_logger_mt<SeverityLevel> {
 public:
  class Ostream {
   public:
    explicit Ostream(Logger &log, SeverityLevel level);
    Ostream(const Ostream &other) = delete;
    Ostream &operator=(const Ostream &other) = delete;
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

  Ostream Trace();
  Ostream Debug();
  Ostream Info();
  Ostream Warning();
  Ostream Error();
  Ostream Fatal();

 private:
  std::shared_ptr<Sink> sink_;
};

}  // namespace call_center::log

#endif  // CALL_CENTER_LOGGER_H
