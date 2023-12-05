#include "severity_level.h"

#include <ostream>

namespace call_center::log {

std::ostream& operator<<(std::ostream& out, const SeverityLevel level) {
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

}  // namespace call_center::log
