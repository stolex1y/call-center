#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <functional>

#include "call_status.h"
#include "configuration.h"
#include "core/queueing_system/request.h"

namespace call_center {

/**
 * @brief В центре обработки вызовов каждый запрос представлен экземпляром данного класса.
 *
 * Помимо различных моментов времени, связанных с обслуживанием вызова, содержит обратный вызов при
 * завершении обработки, а также результат обслуживания.
 */
class CallDetailedRecord : public core::qs::Request {
 public:
  /// Обратный вызов при завершении обслуживания.
  using OnFinish = std::function<void(const CallDetailedRecord &cdr)>;

  /// Ключ в конфигурации, соответствующий значению максимального времени ожидания в секундах.
  static constexpr auto kMaxWaitKey = "call_max_wait";

  CallDetailedRecord(
      std::string caller_phone_number,
      std::shared_ptr<Configuration> configuration,
      OnFinish on_finish
  );
  ~CallDetailedRecord() override = default;

  [[nodiscard]] std::optional<Duration> GetServiceTime() const override;
  [[nodiscard]] std::optional<Duration> GetTotalTime() const override;
  [[nodiscard]] std::optional<TimePoint> GetArrivalTime() const override;
  [[nodiscard]] std::optional<TimePoint> GetServiceCompleteTime() const override;
  [[nodiscard]] std::optional<TimePoint> GetServiceStartTime() const override;
  [[nodiscard]] std::optional<Duration> GetWaitTime() const override;
  [[nodiscard]] bool WasFinished() const override;
  [[nodiscard]] bool WasServiced() const override;
  [[nodiscard]] bool WasArrived() const override;
  [[nodiscard]] bool IsTimeout() const override;

  /**
   * @brief Зафиксировать время прибытия запроса в систему.
   */
  virtual void SetArrivalTime();
  /**
   * @brief Зафиксировать время начала обслуживания.
   * @param operator_id идентификатор оператора, занимающегося обслуживанием
   */
  virtual void StartService(boost::uuids::uuid operator_id);
  /**
   * @brief Зафиксировать время завершения обслуживания.
   * @param status результат обслуживания
   */
  virtual void CompleteService(CallStatus status);
  /**
   * @brief Номер инициатора вызова.
   */
  [[nodiscard]] virtual const std::string &GetCallerPhoneNumber() const;
  /**
   * @brief Результат обработки вызова.
   * @return std::nullopt - если обслуживание не было завершено.
   */
  [[nodiscard]] virtual std::optional<CallStatus> GetStatus() const;
  /**
   * @brief Идентификатор оператора, который занимался обслуживанием звонка.
   * @return std::nullopt - если обслуживание так и не было начато.
   */
  [[nodiscard]] virtual std::optional<boost::uuids::uuid> GetOperatorId() const;
  /**
   * @brief Максимальное время ожидания вызова.
   */
  [[nodiscard]] virtual WaitingDuration GetMaxWait() const;
  /**
   * @brief Время окончания ожидания.
   * @return std::nullopt - если запрос ещё не был получен системой.
   */
  [[nodiscard]] virtual std::optional<TimePoint> GetTimeoutPoint() const;

 protected:
  static constexpr WaitingDuration kDefaultMaxWait_{30};

  mutable std::shared_mutex mutex_;
  const std::shared_ptr<Configuration> configuration_;
  std::optional<TimePoint> arrival_time_;
  std::optional<TimePoint> complete_service_time_;
  std::optional<TimePoint> start_service_time_;
  WaitingDuration max_wait_ = kDefaultMaxWait_;
  std::optional<TimePoint> timeout_point_;
  const std::string caller_phone_number_;
  std::optional<CallStatus> status_ = std::nullopt;
  std::optional<boost::uuids::uuid> operator_id_ = std::nullopt;
  const OnFinish on_finish_;

  /**
   * @brief Прочитать максимальное время ожидания из конфигурации.
   */
  [[nodiscard]] uint64_t ReadMaxWait() const;
  /**
   * @brief Был ли завершен вызов, в отличие от @link WasFinished @endlink не защищен мьютексом.
   */
  [[nodiscard]] bool WasFinished_() const;
  /**
   * @brief Был ли обслужен запрос, в отличие от @link WasServiced @endlink не защищен мьютексом.
   */
  [[nodiscard]] bool WasServiced_() const;
  /**
   * @brief Был ли запрос получен системой, в отличие от @link WasArrived @endlink не защищен
   * мьютексом.
   */
  [[nodiscard]] bool WasArrived_() const;
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
