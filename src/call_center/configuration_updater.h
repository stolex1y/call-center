#ifndef CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_UPDATER_H_
#define CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_UPDATER_H_

#include <chrono>
#include <memory>

#include "configuration.h"
#include "core/tasks/task_manager.h"
#include "log/logger.h"
#include "log/logger_provider.h"

namespace call_center {

/**
 * @brief Класс для запуска регулярных обновлений конфигурации по таймеру.
 */
class ConfigurationUpdater : public std::enable_shared_from_this<ConfigurationUpdater> {
 public:
  static std::shared_ptr<ConfigurationUpdater> Create(
      std::shared_ptr<Configuration> configuration,
      std::shared_ptr<core::tasks::TaskManager> task_manager,
      const log::LoggerProvider &logger_provider
  );

  /**
   * @brief Запустить регулярные обновления.
   */
  void StartUpdating();

 private:
  using Duration = std::chrono::minutes;

  /// Ключ в конфигурации, соответствующий значению времени обновления конфигураии в минутах.
  static constexpr auto kUpdatingPeriodKey_ = "configuration_updating_period";
  static constexpr Duration kDefaultUpdatingPeriod_ = Duration(10);

  std::shared_ptr<core::tasks::TaskManager> task_manager_;
  std::shared_ptr<Configuration> configuration_;
  Duration updating_period_ = kDefaultUpdatingPeriod_;
  std::unique_ptr<log::Logger> logger_;

  explicit ConfigurationUpdater(
      std::shared_ptr<Configuration> configuration,
      std::shared_ptr<core::tasks::TaskManager> task_manager,
      const log::LoggerProvider &logger_provider
  );

  /**
   * @brief Запланировать следующее обновление конфигурации.
   */
  void ScheduleUpdating();
  /**
   * @brief Обновить значение периода обновления конфигурации.
   */
  void UpdateUpdatingPeriod();
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_UPDATER_H_
