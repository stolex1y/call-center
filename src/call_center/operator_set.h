#ifndef CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_
#define CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <unordered_set>

#include "configuration.h"
#include "core/containers/concurrent_hash_set.h"
#include "core/task_manager.h"
#include "operator.h"

namespace call_center {

class OperatorSet {
 public:
  using OperatorProvider = std::function<std::shared_ptr<Operator>()>;

  static constexpr const auto kOperatorCountKey_ = "operator_count";

  OperatorSet(
      std::shared_ptr<Configuration> configuration,
      OperatorProvider operator_provider,
      const log::LoggerProvider &logger_provider
  );
  OperatorSet(const OperatorSet &other) = delete;
  OperatorSet &operator=(const OperatorSet &other) = delete;

  std::shared_ptr<Operator> EraseFree();
  void InsertFree(const std::shared_ptr<Operator> &op);
  [[nodiscard]] size_t GetSize() const;
  [[nodiscard]] size_t GetFreeOperatorCount() const;
  [[nodiscard]] size_t GetBusyOperatorCount() const;

 private:
  using OperatorPtr = std::shared_ptr<Operator>;

  struct OperatorEquals {
    bool operator()(const OperatorPtr &first, const OperatorPtr &second) const;
  };

  struct OperatorHash {
    size_t operator()(const OperatorPtr &op) const;
  };

  static constexpr const size_t kDefaultOperatorCount_ = 10;

  std::unordered_set<OperatorPtr, OperatorHash, OperatorEquals> free_operators_;
  std::unordered_set<OperatorPtr, OperatorHash, OperatorEquals> busy_operators_;
  const std::shared_ptr<Configuration> configuration_;
  size_t operator_count_ = kDefaultOperatorCount_;
  mutable std::shared_mutex mutex_;
  std::unique_ptr<log::Logger> logger_;
  OperatorProvider operator_provider_;

  [[nodiscard]] size_t ReadOperatorCount() const;
  void UpdateOperatorCount();
  void AddOperators(size_t count);
  size_t RemoveOperators(size_t count);
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_
