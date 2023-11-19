#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_

#include <ostream>

namespace call_center::log {

enum class SeverityLevel {
  kTrace,
  kDebug,
  kInfo,
  kWarning,
  kError,
  kFatal
};

std::ostream &operator<<(std::ostream &out, SeverityLevel level);

}

#endif //CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_
