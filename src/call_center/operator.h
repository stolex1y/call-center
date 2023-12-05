#ifndef CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_
#define CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <functional>
#include <memory>
#include <random>

#include "call_detailed_record.h"
#include "configuration.h"
#include "core/queueing_system/server.h"
#include "core/tasks/task_manager.h"

namespace call_center {

/**
 * @brief В центре обработки вызовов обслуживающий прибор представлен экземпляром данного класса,
 * т.е. оператором.
 */
class Operator : public std::enable_shared_from_this<Operator>, public core::qs::Server {
 public:
  /**
   * @brief Состояние оператора.
   */
  enum class Status {
    kFree,  ///< Свободен.
    kBusy   ///< Занят обработкой вызова.
  };

  /// Обратный вызов при завершении обслуживания вызова.
  using OnFinishHandle = std::function<void()>;
  /// Единицы измерения продолжительности обработки вызова.
  using DelayDuration = std::chrono::seconds;

  /// Ключ в конфигурации, соответствующий значению минимальной продолжительности обслуживания в
  /// секундах.
  static constexpr auto kMinDelayKey = "operator_min_delay";
  /// Ключ в конфигурации, соответствующий значению максимальной продолжительности обслуживания в
  /// секундах.
  static constexpr auto kMaxDelayKey = "operator_max_delay";

  static std::shared_ptr<Operator> Create(
      std::shared_ptr<core::tasks::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );

  Operator(const Operator &other) = delete;
  Operator &operator=(const Operator &other) = delete;
  ~Operator() override = default;

  [[nodiscard]] bool IsFree() const override;
  [[nodiscard]] bool IsBusy() const override;

  /**
   * @brief Обработать вызов.
   * @param call вызов
   * @param on_finish обратный вызов при завершении обслуживания
   */
  virtual void HandleCall(
      const std::shared_ptr<CallDetailedRecord> &call, const OnFinishHandle &on_finish
  );

 protected:
  Operator(
      std::shared_ptr<core::tasks::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );

 private:
  using Distribution = std::uniform_int_distribution<uint64_t>;
  using Generator = std::mt19937_64;

  static constexpr uint64_t kDefaultMinDelay_ = 10;
  static constexpr uint64_t kDefaultMaxDelay_ = 60;

  std::atomic<Status> status_ = Status::kFree;
  const std::shared_ptr<core::tasks::TaskManager> task_manager_;
  const std::shared_ptr<Configuration> configuration_;
  uint64_t min_delay_ = kDefaultMinDelay_;
  uint64_t max_delay_ = kDefaultMaxDelay_;
  Generator generator_;
  Distribution distribution_{min_delay_, max_delay_};
  std::unique_ptr<log::Logger> logger_;

  /**
   * @brief Получить значение продолжительности обработки вызова.
   *
   * Значение продолжительности имеет равномерное распределение от @link min_delay_ @endlink до
   * @link max_delay_ @endlink.
   */
  [[nodiscard]] DelayDuration GetCallDelay();
  /**
   * @brief Обновить параметры распределения продолжительности обработки значениями из конфигурации.
   */
  void UpdateDistributionParameters();
  /**
   * @brief Прочитать параметры распределения продолжительности обработки из конфигурации.
   */
  [[nodiscard]] std::pair<uint64_t, uint64_t> ReadMinMax() const;
  /**
   * @brief Проинициализировать распределение продолжительности обработки вызовов.
   */
  void InitDistributionParameters();
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_
