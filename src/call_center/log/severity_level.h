#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_

#include <ostream>

namespace call_center::log {

/**
 * @brief Уровень логирования.
 */
enum class SeverityLevel { kTrace, kDebug, kInfo, kWarning, kError, kFatal };

/**
 * @brief Вывод строкового представления уровня логирования.
 */
std::ostream &operator<<(std::ostream &out, SeverityLevel level);

}  // namespace call_center::log

#endif  // CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_
