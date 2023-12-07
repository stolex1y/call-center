#include "configuration_updater.h"

namespace call_center::config {

std::shared_ptr<ConfigurationUpdater> ConfigurationUpdater::Create(
    std::shared_ptr<Configuration> configuration,
    std::shared_ptr<core::tasks::TaskManager> task_manager,
    const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<ConfigurationUpdater>(
      new ConfigurationUpdater(std::move(configuration), std::move(task_manager), logger_provider)
  );
}

ConfigurationUpdater::ConfigurationUpdater(
    std::shared_ptr<Configuration> configuration,
    std::shared_ptr<core::tasks::TaskManager> task_manager,
    const log::LoggerProvider &logger_provider
)
    : task_manager_(std::move(task_manager)),
      configuration_(std::move(configuration)),
      logger_(logger_provider.Get("ConfigurationUpdater")) {
  UpdateUpdatingPeriod();
}

void ConfigurationUpdater::StartUpdating() {
  ScheduleUpdating();
}

void ConfigurationUpdater::AddUpdateListener(OnUpdate listener) {
  listener(configuration_);
  update_listeners_.emplace_back(std::move(listener));
}

void ConfigurationUpdater::ScheduleUpdating() {
  UpdateUpdatingPeriod();
  logger_->Info() << "Schedule configuration updating after " << updating_period_;
  task_manager_->PostTaskDelayed(updating_period_, [updater = shared_from_this()]() {
    updater->logger_->Info() << "Update configuration";
    updater->configuration_->UpdateConfiguration();
    updater->NotifyListeners();
    updater->ScheduleUpdating();
  });
}

void ConfigurationUpdater::UpdateUpdatingPeriod() {
  updating_period_ =
      Duration(configuration_->GetNumber<uint64_t>(kUpdatingPeriodKey_, updating_period_.count(), 1)
      );
}

void ConfigurationUpdater::NotifyListeners() const {
  for (const auto &listener : update_listeners_) {
    listener(configuration_);
  }
}

}  // namespace call_center::config
