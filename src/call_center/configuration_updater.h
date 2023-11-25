#ifndef CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_UPDATER_H_
#define CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_UPDATER_H_

#include <chrono>
#include <memory>

#include "configuration.h"
#include "core/task_manager.h"
#include "log/logger.h"
#include "log/logger_provider.h"

namespace call_center {

class ConfigurationUpdater : public std::enable_shared_from_this<ConfigurationUpdater> {
 public:
  static std::shared_ptr<ConfigurationUpdater> Create(
      std::shared_ptr<Configuration> configuration,
      std::shared_ptr<core::TaskManager> task_manager,
      const log::LoggerProvider &logger_provider
  );

  void StartUpdating();

 private:
  using Duration = std::chrono::minutes;

  static constexpr const auto kUpdatingPeriodKey_ = "configuration_updating_period";
  static constexpr const Duration kDefaultUpdatingPeriod_ = Duration(10);

  std::shared_ptr<core::TaskManager> task_manager_;
  std::shared_ptr<Configuration> configuration_;
  Duration updating_period_ = kDefaultUpdatingPeriod_;
  std::unique_ptr<log::Logger> logger_;

  explicit ConfigurationUpdater(
      std::shared_ptr<Configuration> configuration,
      std::shared_ptr<core::TaskManager> task_manager,
      const log::LoggerProvider &logger_provider
  );

  void ScheduleUpdating();
  void UpdateUpdatingPeriod();
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CONFIGURATION_UPDATER_H_
