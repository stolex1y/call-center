#ifndef SINK_UPDATER_H
#define SINK_UPDATER_H

#include "configuration/configuration.h"
#include "configuration/configuration_updater.h"

namespace call_center {
/**
 * @brief Основной приемник логов в приложении.
 *
 * Выводит все логи в стандартный поток вывода. Поддерживает обновление уровня логирования из
 * конфигурации.
 */
class MainSink : public log::Sink, public std::enable_shared_from_this<MainSink> {
 public:
  /// Ключ в конфигурации, соответствующий @link SeverityLevel уровню логирования@endlink.
  static constexpr auto kSeverityLevelKey = "log_severity_level";

  static std::shared_ptr<MainSink> Create();

  /**
   * @brief Запустить периодические обновления параметров приёмника из конфигурации.
   */
  void StartSinkUpdate(const std::shared_ptr<config::ConfigurationUpdater> &configuration_updater);

 private:
  MainSink() = default;

  /**
   * @brief Обновить уровень логирования значением из конфигурации.
   */
  void UpdateSeverityLevel(const std::shared_ptr<config::Configuration> &config);
};

}  // namespace call_center

#endif  // SINK_UPDATER_H
