#include "call_center.h"

#include <boost/uuid/uuid_io.hpp>
#include <chrono>

namespace call_center {
using namespace std::chrono_literals;
using namespace qs::metrics;

/**
 * Creating an instance of a class.
 */
std::shared_ptr<CallCenter> CallCenter::Create(
    std::unique_ptr<Journal> journal,
    std::shared_ptr<Configuration> configuration,
    std::shared_ptr<core::TaskManager> task_manager,
    const log::LoggerProvider& logger_provider,
    std::unique_ptr<OperatorSet> operator_set,
    std::unique_ptr<CallQueue> call_queue,
    std::shared_ptr<QueueingSystemMetrics> metrics
) {
  return std::shared_ptr<CallCenter>(new CallCenter(
      std::move(journal),
      std::move(configuration),
      std::move(task_manager),
      logger_provider,
      std::move(operator_set),
      std::move(call_queue),
      std::move(metrics)
  ));
}

CallCenter::CallCenter(
    std::unique_ptr<Journal> journal,
    std::shared_ptr<Configuration> configuration,
    std::shared_ptr<core::TaskManager> task_manager,
    const log::LoggerProvider& logger_provider,
    std::unique_ptr<OperatorSet> operator_set,
    std::unique_ptr<CallQueue> call_queue,
    std::shared_ptr<QueueingSystemMetrics> metrics
)
    : journal_(std::move(journal)),
      operators_(std::move(operator_set)),
      calls_(std::move(call_queue)),
      task_manager_(std::move(task_manager)),
      configuration_(std::move(configuration)),
      logger_(logger_provider.Get("CallCenter")),
      metrics_(std::move(metrics)) {
  metrics_->Start();
}

/**
 * Placing a new call in the queue to processing.
 * @param call - call for processing.
 */
void CallCenter::PushCall(const CallPtr& call) {
  call->SetArrivalTime();
  metrics_->RecordRequestArrival(call);
  // try to bypass the queue
  const auto started = StartCallProcessingIfPossible(call);
  if (started)
    return;

  const auto result = calls_->PushToQueue(call);
  switch (result) {
    case CallQueue::PushResult::kOk: {
      PerformCallProcessingIteration();
      break;
    }
    case CallQueue::PushResult::kAlreadyInQueue: {
      RejectCall(call, CallStatus::kAlreadyInQueue);
      break;
    }
    case CallQueue::PushResult::kOverload: {
      RejectCall(call, CallStatus::kOverload);
      break;
    }
  }
}

/**
 * The main task is to process calls from the queue.
 * Cancels all calls with expired waiting time, after that, if there is a free
 * operator, then sends him a request. Otherwise, it looks at the minimum
 * timeout point of waiting calls and plans a new iteration for this time.
 */
void CallCenter::PerformCallProcessingIteration() {
  if (calls_->QueueIsEmpty())
    return;

  RejectAllTimeoutCalls();

  auto op = operators_->EraseFree();
  if (op) {
    CallPtr call = calls_->PopFromQueue();
    if (call) {
      calls_->InsertToProcessing(call);
      StartCallProcessing(call, op);
    } else {
      operators_->InsertFree(op);
    }
  } else {
    CallPtr call = calls_->GetMinTimeoutCallInQueue();
    if (call) {
      ScheduleCallProcessingIteration(*call->GetTimeoutPoint());
    }
  }
}

/**
 * Start processing the call.
 * @param call - next call from the queue;
 * @param op - free operator who will handle the call.
 */
void CallCenter::StartCallProcessing(const CallPtr& call, const OperatorPtr& op) {
  logger_->Debug() << "Start call (" << boost::uuids::to_string(call->GetId()) << ") processing";
  call->StartService(op->GetId());
  metrics_->RecordServiceStart(call);
  op->HandleCall(call, [call, op, call_center = shared_from_this()]() {
    call_center->FinishCallProcessing(call, op);
  });
}

/**
 * Start processing a call if the queue is empty, there is a free operator and
 * the call is unique. It is necessary to bypass the queue, as it may have zero
 * capacity.
 * @param call - call for processing.
 * @return True if processing has stopped_, otherwise - false.
 */
bool CallCenter::StartCallProcessingIfPossible(const CallPtr& call) {
  if (!calls_->QueueIsEmpty())
    return false;

  auto op = operators_->EraseFree();
  if (op) {
    const auto added = calls_->InsertToProcessing(call);
    if (added) {
      StartCallProcessing(call, op);
      return true;
    } else {
      operators_->InsertFree(op);
      return false;
    }
  }
  return false;
}

/**
 * Finish processing the call.
 * @param call - handled call;
 * @param op - operator who processed the call.
 */
void CallCenter::FinishCallProcessing(const CallPtr& call, const OperatorPtr& op) {
  logger_->Debug() << "Finish call processing (" << boost::uuids::to_string(call->GetId()) << ")";
  call->CompleteService(CallStatus::kOk);
  metrics_->RecordServiceComplete(call, op);
  calls_->EraseFromProcessing(call);
  operators_->InsertFree(op);
  journal_->AddRecord(*call);

  // now there is at least one free operator
  if (!calls_->QueueIsEmpty()) {
    task_manager_->PostTask([call_center = shared_from_this()]() {
      call_center->PerformCallProcessingIteration();
    });
  }
}

/**
 * Schedule a new iteration of call processing at a certain time point.
 * @param time_point - time of execution of the new iteration of call
 * processing.
 */
void CallCenter::ScheduleCallProcessingIteration(const CallDetailedRecord::TimePoint& time_point) {
  logger_->Debug() << "Add task to call processing at: " << time_point;

  const auto task = [call_center = shared_from_this()]() {
    call_center->PerformCallProcessingIteration();
  };
  task_manager_->PostTaskAt(time_point, task);
}

/**
 * Cancel call with for the specified reason.
 * @param call - call to reject;
 * @param reason - reason for rejection.
 */
void CallCenter::RejectCall(const CallPtr& call, CallStatus reason) const {
  logger_->Info() << "Reject call (" << boost::uuids::to_string(call->GetId()) << ") - " << reason;
  call->CompleteService(reason);
  metrics_->RecordRequestDropout(call);
  journal_->AddRecord(*call);
}

/**
 * Cancel all calls with expired waiting time.
 */
void CallCenter::RejectAllTimeoutCalls() const {
  auto to_reject = calls_->EraseTimeoutCallFromQueue();
  while (to_reject) {
    RejectCall(to_reject, CallStatus::kTimeout);
    to_reject = calls_->EraseTimeoutCallFromQueue();
  }
}
}  // namespace call_center
