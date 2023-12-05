#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_STATUS_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_STATUS_H_

#include <string>

namespace call_center {

/**
 * @brief Статус обработки вызова.
 */
enum class CallStatus {
  kOk,        ///< Успешная обработка вызова.
  kOverload,  ///< Вызов отклонен по причине максимальной загруженности очереди.
  kTimeout,  ///< Вызов отклонен по причине превышения времени ожидания.
  kAlreadyInQueue  ///< Вызов отклонен по причине наличия такого же вызова в очереди либо на
                   ///< обслуживании.
};

/**
 * @brief Текстовое представление статуса.
 */
std::string to_string(CallStatus status);
/**
 * @brief Вывод текстового представления статуса в поток std::ostream.
 */
std::ostream &operator<<(std::ostream &out, CallStatus status);

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_STATUS_H_
