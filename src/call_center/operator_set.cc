#include "operator_set.h"

#include <boost/uuid/uuid.hpp>
#include <boost/functional/hash.hpp>

namespace call_center {

OperatorSet::OperatorSet(const Configuration &configuration,
                         core::TaskManager &task_manager)
    : configuration_(configuration), operator_count_(ReadOperatorCount()),
      task_manager_(task_manager) {
  AddOperators(operator_count_);
}

std::shared_ptr<Operator> OperatorSet::EraseFree() {
  std::lock_guard<std::mutex> lock(mutex_);

  UpdateOperatorCount();

  if (free_operators_.empty())
    return nullptr;

  auto erased = *free_operators_.begin();
  free_operators_.erase(free_operators_.begin());
  return erased;
}

void OperatorSet::InsertFree(const std::shared_ptr<Operator> &op) {
  std::lock_guard<std::mutex> lock(mutex_);
  free_operators_.emplace(op);
  UpdateOperatorCount();
}

size_t OperatorSet::ReadOperatorCount() const {
  return configuration_.GetProperty<size_t>(kOperatorCountKey).value_or(kDefaultOperatorCount);
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
    free_operators_.emplace(Operator::Create(task_manager_, configuration_));
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

bool OperatorSet::OperatorEquals::operator()(const OperatorPtr &first,
                                             const OperatorPtr &second) const {
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

} // call_center
