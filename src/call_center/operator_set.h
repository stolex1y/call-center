#ifndef CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_
#define CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <unordered_set>

#include "configuration/configuration.h"
#include "core/containers/concurrent_hash_set.h"
#include "core/queueing_system/metrics/queueing_system_metrics.h"
#include "core/utils/uuids.h"
#include "operator.h"

namespace call_center {

/**
 * @brief Множество операторов.
 */
class OperatorSet {
 public:
  using OperatorPtr = std::shared_ptr<Operator>;
  using OperatorProvider = std::function<OperatorPtr()>;

  /// Ключ в конфигурации, соответствующий значению количества обслуживающих операторов.
  static constexpr auto kOperatorCountKey = "operator_count";

  OperatorSet(
      std::shared_ptr<config::Configuration> configuration,
      OperatorProvider operator_provider,
      const log::LoggerProvider &logger_provider,
      std::shared_ptr<core::qs::metrics::QueueingSystemMetrics> metrics
  );
  OperatorSet(const OperatorSet &other) = delete;
  OperatorSet &operator=(const OperatorSet &other) = delete;

  /**
   * @brief Получить любого свободного оператора из множеста.
   * @return std::nullopt - если свободных операторов нет.
   */
  std::shared_ptr<Operator> EraseFree();
  /**
   * @brief Вернуть освободившегося оператора в множество.
   */
  void InsertFree(const OperatorPtr &op);
  /**
   * @brief Общее количество операторов (свободных и занятых).
   */
  [[nodiscard]] size_t GetSize() const;
  /**
   * @brief Количество свободных на данный момент операторов.
   */
  [[nodiscard]] size_t GetFreeOperatorCount() const;
  /**
   * @brief Количество занятых на данный момент операторов.
   */
  [[nodiscard]] size_t GetBusyOperatorCount() const;

 private:
  struct OperatorEquals {
    bool operator()(const OperatorPtr &first, const OperatorPtr &second) const;
  };

  struct OperatorHash {
    size_t operator()(const OperatorPtr &op) const;
  };

  static constexpr size_t kDefaultOperatorCount_ = 10;

  std::unordered_set<OperatorPtr, OperatorHash, OperatorEquals> free_operators_;
  std::unordered_set<OperatorPtr, OperatorHash, OperatorEquals> operators_;
  const std::shared_ptr<config::Configuration> configuration_;
  mutable std::shared_mutex mutex_;
  std::unique_ptr<log::Logger> logger_;
  OperatorProvider operator_provider_;
  std::shared_ptr<core::qs::metrics::QueueingSystemMetrics> metrics_;

  /**
   * @brief Прочитать значение количества операторов в множестве из конфигурации.
   */
  [[nodiscard]] size_t ReadOperatorCount(size_t default_value = kDefaultOperatorCount_) const;
  /**
   * @brief Обновить значение количества операторов в множестве, согласно конфигурации.
   */
  void UpdateOperatorCount();
  /**
   * @brief Добавить заданное количество операторов в множество.
   */
  void AddOperators(size_t count);
  /**
   * @brief Удалить заданное количество операторов из множества.
   */
  void RemoveOperators(size_t count);
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_
