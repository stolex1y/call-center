#ifndef CALL_CENTER_LOGGER_H
#define CALL_CENTER_LOGGER_H

#include <boost/log/core.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <string>
#include <optional>

namespace call_center::log {

enum class SeverityLevel {
  kTrace,
  kDebug,
  kInfo,
  kWarning,
  kError,
  kFatal
};

class Logger : private boost::log::sources::severity_logger_mt<SeverityLevel> {
 public:
  class Ostream : private boost::log::record_ostream {
   public:
    explicit Ostream(Logger &log, SeverityLevel level);
    ~Ostream();

    using boost::log::record_ostream::operator<<;

   private:
    Logger &logger_;
    boost::log::record record_;
  };

  static void SetLevel(SeverityLevel level);
  static void SetFileName(const std::string &file_name);
  static void SetMaxSize(int max_size);

  explicit Logger(std::string tag);

  Ostream Trace();
  Ostream Debug();
  Ostream Info();
  Ostream Warning();
  Ostream Error();
  Ostream Fatal();

 private:
  static SeverityLevel level_;
  static std::string file_name_;
  static int max_size_;

  static void Formatter(const boost::log::record_view &rec, boost::log::formatting_ostream &out);
  static void UpdateLogger();

  std::string tag_;
};

}

#endif //CALL_CENTER_LOGGER_H
