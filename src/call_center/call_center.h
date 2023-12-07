#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_

#include <chrono>

#include "call_detailed_record.h"
#include "call_queue.h"
#include "configuration/configuration.h"
#include "core/containers/concurrent_hash_set.h"
#include "core/queueing_system/metrics/queueing_system_metrics.h"
#include "core/tasks/task_manager.h"
#include "journal.h"
#include "log/logger.h"
#include "log/logger_provider.h"
#include "operator_set.h"

namespace call_center {

using namespace core;

/**
 * @brief Класс собирающий внутри себя все части центра обработки вызовов.
 *
 * Класс собирающий внутри себя все части центра обработки вызовов: очередь вызовов,
 * множество операторов, планировщик задач, метрики.
 */
class CallCenter : public std::enable_shared_from_this<CallCenter> {
 public:
  using CallPtr = std::shared_ptr<CallDetailedRecord>;
  using OperatorPtr = std::shared_ptr<Operator>;

  static std::shared_ptr<CallCenter> Create(
      std::unique_ptr<Journal> journal,
      std::shared_ptr<config::Configuration> configuration,
      std::shared_ptr<tasks::TaskManager> task_manager,
      const log::LoggerProvider &logger_provider,
      std::unique_ptr<OperatorSet> operator_set,
      std::unique_ptr<CallQueue> call_queue,
      std::shared_ptr<qs::metrics::QueueingSystemMetrics> metrics
  );

  CallCenter(const CallCenter &other) = delete;
  CallCenter &operator=(const CallCenter &other) = delete;
  virtual ~CallCenter() = default;

  /**
   * @brief Поместить новый вызов в очередь на выполнение.
   */
  void PushCall(const CallPtr &call);

 protected:
  CallCenter(
      std::unique_ptr<Journal> journal,
      std::shared_ptr<config::Configuration> configuration,
      std::shared_ptr<tasks::TaskManager> task_manager,
      const log::LoggerProvider &logger_provider,
      std::unique_ptr<OperatorSet> operator_set,
      std::unique_ptr<CallQueue> call_queue,
      std::shared_ptr<qs::metrics::QueueingSystemMetrics> metrics
  );

 private:
  const std::unique_ptr<Journal> journal_;
  const std::unique_ptr<OperatorSet> operators_;
  const std::unique_ptr<CallQueue> calls_;
  const std::shared_ptr<tasks::TaskManager> task_manager_;
  const std::shared_ptr<config::Configuration> configuration_;
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<qs::metrics::QueueingSystemMetrics> metrics_;

  /**
   * @brief Выполнить очередную итерацию обработки вызовов в очереди.
   *
   * Основная цель метода - обработка звонков, находящихся в очереди. Для этого сначала проверяется,
   * есть ли в очереди звонки, время ожидания которых вышло. Если есть, то они отклоняются.
   * После этого, если есть свободный оператор, то направить первый вызов из очереди выбранному
   * оператору. Иначе выбирается минимальное время окночания ожидания по всем вызовам в очереди и
   * планируется новая итерация на это время.
   */
  void PerformCallProcessingIteration();
  /**
   * @brief Завершить обработку вызова.
   * @param call обработанный вызов
   * @param op оператор, обслуживающий вызов
   */
  void FinishCallProcessing(const CallPtr &call, const OperatorPtr &op);
  /**
   * @brief Начать обработку вызова, если очередь пуста и есть свободный оператор.
   *
   * Это необходимо, чтобы миновать очередь, которая может иметь нулевую емкость.
   * @param call call for processing
   * @return true - если обработка началась, иначе - false.
   */
  bool StartCallProcessingIfPossible(const CallPtr &call);
  /**
   * @brief Начать обработку вызова.
   * @param call - вызов для обработки
   * @param op - свободный оператор
   */
  void StartCallProcessing(const CallPtr &call, const OperatorPtr &op);
  /**
   * @brief Запланировать выполнение новой итерации по обработке вызовов в определенный момент
   * времени.
   */
  void ScheduleCallProcessingIteration(const CallDetailedRecord::TimePoint &time_point);
  /**
   * @brief Отклонить вызов по определенной причине.
   * @param call отклоненный вызов
   * @param reason причина отклонения
   */
  void RejectCall(const CallPtr &call, CallStatus reason) const;
  /**
   * @brief Отклонить все вызовы, время ожидания которых истекло.
   */
  void RejectAllTimeoutCalls() const;
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_
