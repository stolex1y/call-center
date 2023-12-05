#ifndef FAKE_SERVICE_LOADER_H
#define FAKE_SERVICE_LOADER_H

#include <random>

#include "call_center.h"

namespace call_center::test {

/**
 * @brief Нагрузчик для класса @link CallCenter @endlink.
 *
 * Позволяет нагрузить систему, используя предоставленный провайдер запросов.
 */
class FakeServiceLoader : public std::enable_shared_from_this<FakeServiceLoader> {
 public:
  /// Провайдер запросов.
  using CallProvider = std::function<std::shared_ptr<CallDetailedRecord>()>;

  static std::shared_ptr<FakeServiceLoader> Create(
      std::shared_ptr<tasks::TaskManager> task_manager,
      std::shared_ptr<CallCenter> call_center,
      CallProvider call_provider
  );

  /**
   * @brief Начать отправку запросов.
   *
   * Интервал между запросами представляет собой экспоненциальное распределение.
   *
   * @param lambda параметр экспоненциального распределения
   */
  void StartLoading(double lambda);

 private:
  using Distribution = std::exponential_distribution<>;
  using Generator = std::mt19937_64;
  using DelayDuration = std::chrono::duration<double>;

  std::shared_ptr<tasks::TaskManager> task_manager_;
  std::shared_ptr<CallCenter> call_center_;
  Generator generator_;
  Distribution distribution_;
  CallProvider call_provider_;

  FakeServiceLoader(
      std::shared_ptr<tasks::TaskManager> task_manager,
      std::shared_ptr<CallCenter> call_center,
      CallProvider call_provider
  );

  void SchedulePostCall();
};

}  // namespace call_center::test

#endif  // FAKE_SERVICE_LOADER_H
