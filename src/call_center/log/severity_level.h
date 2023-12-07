#ifndef CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_
#define CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_

#include <optional>
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
/**
 * @brief Преобразование из @link SeverityLevel @endlink в строку.
 */
std::string to_string(SeverityLevel level);
/**
 * @brief Преобразование из строки в @link SeverityLevel @endlink.
 * @return std::nullopt - если константа не найдена.
 */
std::optional<SeverityLevel> ParseSeverityLevel(std::string str);

}  // namespace call_center::log

#endif  // CALL_CENTER_SRC_CALL_CENTER_LOG_SEVERITY_LEVEL_H_
