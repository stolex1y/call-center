#ifndef CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_
#define CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <functional>
#include <memory>
#include <random>

#include "call_detailed_record.h"
#include "configuration.h"
#include "core/task_manager.h"

namespace call_center {

class Operator : public std::enable_shared_from_this<Operator> {
 public:
  enum class Status { kFree, kBusy };

  using OnFinishHandle = std::function<void()>;

  static std::shared_ptr<Operator> Create(
      std::shared_ptr<core::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );

  Operator(const Operator &other) = delete;
  Operator &operator=(const Operator &other) = delete;

  [[nodiscard]] const boost::uuids::uuid &GetId() const;
  [[nodiscard]] Operator::Status GetStatus() const;

  void HandleCall(const std::shared_ptr<CallDetailedRecord> &call, const OnFinishHandle &on_finish);

 private:
  using Distribution = std::uniform_int_distribution<uint64_t>;
  using Generator = std::mt19937_64;
  using DelayDuration = std::chrono::seconds;

  static constexpr const auto kMinDelayKey_ = "operator_min_delay";
  static constexpr const auto kMaxDelayKey_ = "operator_max_delay";
  static constexpr const uint64_t kDefaultMinDelay_ = 10;
  static constexpr const uint64_t kDefaultMaxDelay_ = 60;

  const boost::uuids::uuid id_ = boost::uuids::random_generator_mt19937()();
  Status status_ = Status::kFree;
  mutable std::mutex mutex_;
  const std::shared_ptr<core::TaskManager> task_manager_;
  const std::shared_ptr<Configuration> configuration_;
  uint64_t min_delay_ = kDefaultMinDelay_;
  uint64_t max_delay_ = kDefaultMaxDelay_;
  Generator generator_;
  Distribution distribution_{min_delay_, max_delay_};
  std::unique_ptr<log::Logger> logger_;

  Operator(
      std::shared_ptr<core::TaskManager> task_manager,
      std::shared_ptr<Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );

  [[nodiscard]] DelayDuration GetCallDelay();
  void UpdateDistributionParameters();
  [[nodiscard]] std::pair<uint64_t, uint64_t> ReadMinMax() const;
  void InitDistributionParameters();
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_OPERATOR_H_
