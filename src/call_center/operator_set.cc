#include "operator_set.h"

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>

namespace call_center {

using namespace core::utils::uuids;
using namespace core::qs::metrics;

OperatorSet::OperatorSet(
    std::shared_ptr<Configuration> configuration,
    OperatorProvider operator_provider,
    const log::LoggerProvider &logger_provider,
    std::shared_ptr<QueueingSystemMetrics> metrics
)
    : configuration_(std::move(configuration)),
      logger_(logger_provider.Get("OperatorSet")),
      operator_provider_(std::move(operator_provider)),
      metrics_(std::move(metrics)) {
  AddOperators(ReadOperatorCount());
  metrics_->SetServers(operators_);
}

std::shared_ptr<Operator> OperatorSet::EraseFree() {
  std::lock_guard lock(mutex_);

  UpdateOperatorCount();
  if (free_operators_.empty())
    return nullptr;

  auto erased = *free_operators_.begin();
  free_operators_.erase(free_operators_.begin());
  logger_->Debug() << "Take free operator " << erased->GetId();
  return erased;
}

void OperatorSet::InsertFree(const std::shared_ptr<Operator> &op) {
  std::lock_guard lock(mutex_);
  if (!operators_.contains(op)) {
    logger_->Warning() << "Unknown operator " << op->GetId();
    return;
  }
  logger_->Debug() << "Return free operator " << op->GetId();
  free_operators_.emplace(op);
  UpdateOperatorCount();
}

size_t OperatorSet::ReadOperatorCount(const size_t default_value) const {
  return configuration_->GetNumber<size_t>(kOperatorCountKey, default_value, 1);
}

size_t OperatorSet::GetSize() const {
  std::shared_lock lock(mutex_);
  return operators_.size();
}

size_t OperatorSet::GetFreeOperatorCount() const {
  std::shared_lock lock(mutex_);
  return free_operators_.size();
}

size_t OperatorSet::GetBusyOperatorCount() const {
  std::shared_lock lock(mutex_);
  return operators_.size() - free_operators_.size();
}

void OperatorSet::UpdateOperatorCount() {
  const auto cur_count = operators_.size();
  const size_t new_count = ReadOperatorCount(cur_count);
  if (new_count != cur_count) {
    if (new_count > cur_count) {
      const auto to_add = new_count - cur_count;
      AddOperators(to_add);
    } else {
      const auto to_remove = cur_count - new_count;
      RemoveOperators(to_remove);
    }
    metrics_->SetServers(free_operators_);
  }
}

void OperatorSet::AddOperators(const size_t count) {
  for (size_t i = 0; i < count; ++i) {
    const auto op = operator_provider_();
    operators_.emplace(op);
    free_operators_.emplace(op);
  }
}

void OperatorSet::RemoveOperators(const size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (free_operators_.empty()) {
      break;
    }
    const auto erased = *free_operators_.begin();
    operators_.erase(erased);
    free_operators_.erase(erased);
  }
}

bool OperatorSet::OperatorEquals::operator()(const OperatorPtr &first, const OperatorPtr &second)
    const {
  if ((first == nullptr) ^ (second == nullptr))
    return false;

  if (first == nullptr)
    return true;

  return first->GetId() == second->GetId();
}

size_t OperatorSet::OperatorHash::operator()(const OperatorPtr &op) const {
  return op->hash();
}

}  // namespace call_center
