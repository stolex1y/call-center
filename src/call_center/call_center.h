#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_

#include <chrono>

#include "configuration.h"
#include "operator_set.h"
#include "call_queue.h"
#include "journal.h"
#include "log/logger.h"
#include "call_detailed_record.h"
#include "core/task_manager.h"
#include "log/sink.h"
#include "core/containers/concurrent_hash_set.h"

namespace call_center {

class CallCenter : public std::enable_shared_from_this<CallCenter> {
 public:
  using CallPtr = std::shared_ptr<CallDetailedRecord>;
  using OperatorPtr = std::shared_ptr<Operator>;

  static std::shared_ptr<CallCenter> Create(const Configuration &configuration,
                                            core::TaskManager &task_manager,
                                            const log::Sink &sink);

  CallCenter(const CallCenter &other) = delete;
  CallCenter &operator=(const CallCenter &other) = delete;

  void PushCall(const CallPtr &call);

 private:
  Journal journal_;
  OperatorSet operators_;
  CallQueue calls_;
  core::TaskManager &task_manager_;
  const Configuration &configuration_;
  log::Logger logger_;

  CallCenter(const Configuration &configuration, core::TaskManager &task_manager,
             const log::Sink &sink);

  void PerformCallProcessingIteration();
  void FinishCallProcessing(const CallPtr &call, const OperatorPtr &op);
  bool StartCallProcessingIfPossible(const CallPtr &call);
  void StartCallProcessing(const CallPtr &call,
                           const OperatorPtr &op);

  void ScheduleCallProcessingIteration(const CallDetailedRecord::TimePoint &time_point);
  void RejectCall(const CallPtr &call, CallStatus reason);
  void RejectAllTimeoutCalls();
};

} // call_center

#endif //CALL_CENTER_SRC_CALL_CENTER_CALL_CENTER_H_
