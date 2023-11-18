#ifndef CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_
#define CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <memory>
#include <functional>
#include <random>

#include "call_detailed_record.h"
#include "core/task_manager.h"
#include "configuration.h"

namespace call_center {

class Operator : public std::enable_shared_from_this<Operator> {
 public:
  enum class Status {
    kFree,
    kBusy
  };

  using OnFinishHandle = std::function<void()>;

  static std::shared_ptr<Operator> Create(core::TaskManager &task_manager, const Configuration &configuration);

  [[nodiscard]] const boost::uuids::uuid &GetId() const;
  [[nodiscard]] Operator::Status GetStatus() const;

  void HandleCall(const std::shared_ptr<CallDetailedRecord> &call,
                  const OnFinishHandle &on_finish);

 private:
  using Distribution = std::uniform_int_distribution<uint64_t>;
  using Generator = std::mt19937_64;
  using DelayDuration = std::chrono::seconds;

  static constexpr const auto kMinDelayKey = "operator_min_delay";
  static constexpr const auto kMaxDelayKey = "operator_max_delay";
  static constexpr const uint64_t kDefaultMinDelay = 10;
  static constexpr const uint64_t kDefaultMaxDelay = 10;

  boost::uuids::uuid id_ = boost::uuids::random_generator_mt19937()();
  Status status_ = Status::kFree;
  mutable std::mutex mutex_;
  core::TaskManager &task_manager_;
  const Configuration &configuration_;
  uint64_t min_delay_;
  uint64_t max_delay_;
  Generator generator_;
  Distribution distribution_;

  Operator(core::TaskManager &task_manager, const Configuration &configuration);

  uint64_t ReadMinDelay() const;
  uint64_t ReadMaxDelay() const;
  uint64_t GetCallDelay();
  void UpdateDistribution();
};

} // call_center

#endif //CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_
