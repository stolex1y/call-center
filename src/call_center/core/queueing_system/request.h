#ifndef REQUEST_H
#define REQUEST_H

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <optional>

/// Базовые классы для системы массового обслуживания (СМО).
namespace call_center::core::qs {

/**
 * @brief Базовый класс запроса в СМО.
 *
 * Содержит различные временные точки, связанные с поступление, началом обслуживания и др.
 */
class Request {
 public:
  using Id = boost::uuids::uuid;
  using Clock = std::chrono::utc_clock;
  /**
   * @brief Точность измерения временных точек.
   */
  using Duration = std::chrono::milliseconds;
  /**
   * @brief Точность измерения времени ожидания.
   */
  using WaitingDuration = std::chrono::seconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  explicit Request(Id id = boost::uuids::random_generator_mt19937()());
  virtual ~Request() = default;

  /**
   * @brief Время обслуживания в системе.
   * @return std::nullopt - если обслуживание не было начато или завершено
   */
  [[nodiscard]] virtual std::optional<Duration> GetServiceTime() const = 0;
  /**
   * @brief Время пребывания в системе.
   * @return std::nullopt - если запрос не был завершен
   */
  [[nodiscard]] virtual std::optional<Duration> GetTotalTime() const = 0;
  /**
   * @brief Время прибытия в систему.
   * @return std::nullopt - если получение запроса не было зафиксировано
   */
  [[nodiscard]] virtual std::optional<TimePoint> GetArrivalTime() const = 0;
  /**
   * @brief Время завершения обслуживания в системе.
   * @return std::nullopt - если обслуживание не было начато или завершено
   */
  [[nodiscard]] virtual std::optional<TimePoint> GetServiceCompleteTime() const = 0;
  /**
   * @brief Время начала обслуживания в системе.
   * @return std::nullopt - если обслуживание не было начато
   */
  [[nodiscard]] virtual std::optional<TimePoint> GetServiceStartTime() const = 0;
  /**
   * @brief Время ожидания в очереди в системе.
   * @return std::nullopt - если обслуживание не было начато, или запрос не был получен
   */
  [[nodiscard]] virtual std::optional<Duration> GetWaitTime() const = 0;
  /**
   * @brief Завершен ли запрос, при этом это не всегда означает успешное обслуживание в системе.
   * Запрос может быть отклонен по некоторым причинам.
   */
  [[nodiscard]] virtual bool WasFinished() const = 0;
  /**
   * @brief Обслужен ли запрос.
   */
  [[nodiscard]] virtual bool WasServiced() const = 0;
  /**
   * @brief Получен ли запрос системой.
   */
  [[nodiscard]] virtual bool WasArrived() const = 0;
  /**
   * @brief Вышло ли время ожидания в очереди.
   */
  [[nodiscard]] virtual bool IsTimeout() const = 0;

  [[nodiscard]] virtual size_t hash() const;

  [[nodiscard]] bool operator==(const Request &other) const;
  /**
   * @brief Уникальный идентификатор запроса.
   */
  [[nodiscard]] Id GetId() const;

 protected:
  const Id id_;
};

}  // namespace call_center::core::qs

#endif  // REQUEST_H
