#include "severity_level.h"

#include <algorithm>
#include <ostream>

namespace call_center::log {

std::ostream& operator<<(std::ostream& out, const SeverityLevel level) {
  out << to_string(level);
  return out;
}

std::string to_string(const SeverityLevel level) {
  switch (level) {
    case SeverityLevel::kTrace: {
      return "TRACE";
    }
    case SeverityLevel::kDebug: {
      return "DEBUG";
    }
    case SeverityLevel::kInfo: {
      return "INFO";
    }
    case SeverityLevel::kWarning: {
      return "WARNING";
    }
    case SeverityLevel::kError: {
      return "ERROR";
    }
    case SeverityLevel::kFatal:
    default: {
      return "FATAL";
    }
  }
}

std::optional<SeverityLevel> ParseSeverityLevel(std::string str) {
  std::ranges::transform(str, str.begin(), toupper);
  if (str == "TRACE") {
    return SeverityLevel::kTrace;
  }
  if (str == "DEBUG") {
    return SeverityLevel::kDebug;
  }
  if (str == "INFO") {
    return SeverityLevel::kInfo;
  }
  if (str == "WARNING") {
    return SeverityLevel::kWarning;
  }
  if (str == "ERROR") {
    return SeverityLevel::kError;
  }
  if (str == "FATAL") {
    return SeverityLevel::kFatal;
  }
  return std::nullopt;
}

}  // namespace call_center::log
