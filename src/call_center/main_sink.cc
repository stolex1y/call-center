#include "main_sink.h"

namespace call_center {

std::shared_ptr<MainSink> MainSink::Create() {
  return std::shared_ptr<MainSink>(new MainSink());
}

void MainSink::StartSinkUpdate(
    const std::shared_ptr<config::ConfigurationUpdater> &configuration_updater
) {
  configuration_updater->AddUpdateListener([sink = shared_from_this()](const auto &config) {
    return sink->UpdateSeverityLevel(config);
  });
}

void MainSink::UpdateSeverityLevel(const std::shared_ptr<config::Configuration> &config) {
  const auto default_level = GetSeverityLevel();
  const auto new_level =
      config->GetProperty<std::string>(kSeverityLevelKey, to_string(default_level));
  SetSeverityLevel(log::ParseSeverityLevel(new_level).value_or(default_level));
}

}  // namespace call_center
