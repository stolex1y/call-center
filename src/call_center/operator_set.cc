#include "operator_set.h"

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>

#include "core/utils/uuids.h"

namespace call_center {

using namespace core::utils;

OperatorSet::OperatorSet(
    std::shared_ptr<Configuration> configuration,
    OperatorProvider operator_provider,
    const log::LoggerProvider &logger_provider
)
    : configuration_(std::move(configuration)),
      logger_(logger_provider.Get("OperatorSet")),
      operator_provider_(std::move(operator_provider)) {
  operator_count_ = ReadOperatorCount();
  AddOperators(operator_count_);
}

std::shared_ptr<Operator> OperatorSet::EraseFree() {
  std::lock_guard lock(mutex_);

  if (free_operators_.empty())
    return nullptr;

  auto erased = *free_operators_.begin();
  busy_operators_.emplace(erased);
  free_operators_.erase(free_operators_.begin());
  logger_->Debug() << "Take free operator " << erased->GetId();
  UpdateOperatorCount();
  return erased;
}

void OperatorSet::InsertFree(const std::shared_ptr<Operator> &op) {
  std::lock_guard lock(mutex_);
  if (!busy_operators_.contains(op)) {
    logger_->Warning() << "Unknown operator " << op->GetId();
    return;
  }
  logger_->Debug() << "Return free operator " << op->GetId();
  free_operators_.emplace(op);
  busy_operators_.erase(op);
  UpdateOperatorCount();
}

size_t OperatorSet::ReadOperatorCount() const {
  return configuration_->GetNumber<size_t>(kOperatorCountKey_, operator_count_, 1);
}

size_t OperatorSet::GetSize() const {
  std::shared_lock lock(mutex_);
  return operator_count_;
}

size_t OperatorSet::GetFreeOperatorCount() const {
  std::shared_lock lock(mutex_);
  return free_operators_.size();
}

size_t OperatorSet::GetBusyOperatorCount() const {
  std::shared_lock lock(mutex_);
  return busy_operators_.size();
}

void OperatorSet::UpdateOperatorCount() {
  const size_t new_count = ReadOperatorCount();
  if (new_count != operator_count_) {
    if (new_count > operator_count_) {
      const auto to_add = new_count - operator_count_;
      AddOperators(to_add);
      operator_count_ = new_count;
    } else {
      const auto to_remove = operator_count_ - new_count;
      const auto removed = RemoveOperators(to_remove);
      operator_count_ -= removed;
    }
  }
}

void OperatorSet::AddOperators(size_t count) {
  for (size_t i = 0; i < count; ++i) {
    free_operators_.emplace(operator_provider_());
  }
}

size_t OperatorSet::RemoveOperators(size_t count) {
  size_t removed = 0;
  for (size_t i = 0; i < count; ++i) {
    if (free_operators_.empty()) {
      break;
    }
    free_operators_.erase(free_operators_.begin());
    ++removed;
  }
  return removed;
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
  assert(op);
  return boost::hash<boost::uuids::uuid>()(op->GetId());
}

}  // namespace call_center
