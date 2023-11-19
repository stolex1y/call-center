#ifndef CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_
#define CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>

#include "core/containers/concurrent_hash_set.h"
#include "operator.h"
#include "core/task_manager.h"
#include "configuration.h"

namespace call_center {

class OperatorSet {
 public:
  OperatorSet(std::shared_ptr<const Configuration> configuration,
              std::shared_ptr<core::TaskManager> task_manager,
              const std::shared_ptr<const log::LoggerProvider> &logger_provider);
  OperatorSet(const OperatorSet &other) = delete;
  OperatorSet &operator=(const OperatorSet &other) = delete;

  std::shared_ptr<Operator> EraseFree();
  void InsertFree(const std::shared_ptr<Operator> &op);

 private:
  using OperatorPtr = std::shared_ptr<Operator>;

  struct OperatorEquals {
    bool operator()(const OperatorPtr &first, const OperatorPtr &second) const;
  };

  struct OperatorHash {
    size_t operator()(const OperatorPtr &op) const;
  };

  static constexpr const auto kOperatorCountKey = "operator_count";
  static constexpr const size_t kDefaultOperatorCount = 2;

  std::unordered_set<OperatorPtr, OperatorHash, OperatorEquals> free_operators_;
  std::unordered_set<OperatorPtr, OperatorHash, OperatorEquals> busy_operators_;
  const std::shared_ptr<const Configuration> configuration_;
  const std::shared_ptr<core::TaskManager> task_manager_;
  size_t operator_count_;
  mutable std::mutex mutex_;
  const std::shared_ptr<const log::LoggerProvider> &logger_provider_;
  std::unique_ptr<log::Logger> logger_;

  [[nodiscard]] size_t ReadOperatorCount() const;
  void UpdateOperatorCount();
  void AddOperators(size_t count);
  size_t RemoveOperators(size_t count);
};

} // call_center

#endif //CALL_CENTER_SRC_CALL_CENTER_OPERATOR_SET_H_
