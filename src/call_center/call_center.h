#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_

#include <chrono>

#include "call_detailed_record.h"
#include "call_queue.h"
#include "configuration.h"
#include "core/containers/concurrent_hash_set.h"
#include "core/task_manager.h"
#include "journal.h"
#include "log/logger.h"
#include "log/logger_provider.h"
#include "operator_set.h"

namespace call_center {

class CallCenter : public std::enable_shared_from_this<CallCenter> {
 public:
  using CallPtr = std::shared_ptr<CallDetailedRecord>;
  using OperatorPtr = std::shared_ptr<Operator>;

  static std::shared_ptr<CallCenter> Create(
      std::unique_ptr<Journal> journal,
      std::shared_ptr<Configuration> configuration,
      std::shared_ptr<core::TaskManager> task_manager,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider,
      std::unique_ptr<OperatorSet> operator_set,
      std::unique_ptr<CallQueue> call_queue
  );

  CallCenter(const CallCenter &other) = delete;
  CallCenter &operator=(const CallCenter &other) = delete;
  virtual ~CallCenter() = default;

  void PushCall(const CallPtr &call);

 protected:
  CallCenter(
      std::unique_ptr<Journal> journal,
      std::shared_ptr<Configuration> configuration,
      std::shared_ptr<core::TaskManager> task_manager,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider,
      std::unique_ptr<OperatorSet> operator_set,
      std::unique_ptr<CallQueue> call_queue
  );

 private:
  const std::unique_ptr<Journal> journal_;
  const std::unique_ptr<OperatorSet> operators_;
  const std::unique_ptr<CallQueue> calls_;
  const std::shared_ptr<core::TaskManager> task_manager_;
  const std::shared_ptr<Configuration> configuration_;
  const std::unique_ptr<log::Logger> logger_;

  void PerformCallProcessingIteration();
  void FinishCallProcessing(const CallPtr &call, const OperatorPtr &op);
  bool StartCallProcessingIfPossible(const CallPtr &call);
  void StartCallProcessing(const CallPtr &call, const OperatorPtr &op);

  void ScheduleCallProcessingIteration(
      const CallDetailedRecord::TimePoint &time_point
  );
  void RejectCall(const CallPtr &call, CallStatus reason);
  void RejectAllTimeoutCalls();
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_
