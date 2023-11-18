#include "call_center.h"

#include <boost/uuid/uuid_io.hpp>

#include <chrono>

namespace call_center {

using namespace std::chrono_literals;

/**
 * Creating an instance of a class.
 */
std::shared_ptr<CallCenter> CallCenter::Create(const Configuration &configuration,
                                               core::TaskManager &task_manager,
                                               const log::Sink &sink) {
  return std::shared_ptr<CallCenter>(new CallCenter(configuration, task_manager, sink));
}

CallCenter::CallCenter(const Configuration &configuration,
                       core::TaskManager &task_manager,
                       const log::Sink &sink)
    : journal_(configuration), operators_(configuration, task_manager),
      calls_(configuration), task_manager_(task_manager),
      configuration_(configuration), logger_("CallCenter", sink) {
}

/**
 * Placing a new call in the queue to processing.
 * @param call - call for processing.
 */
void CallCenter::PushCall(const CallPtr &call) {
  // try to bypass the queue
  const auto started = StartCallProcessingIfPossible(call);
  if (started)
    return;

  const auto result = calls_.PushToQueue(call);
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
 * Cancels all calls with expired waiting time, after that, if there is a free operator, then sends him a request.
 * Otherwise, it looks at the minimum timeout point of waiting calls and plans a new iteration for this time.
 */
void CallCenter::PerformCallProcessingIteration() {
  if (calls_.QueueIsEmpty())
    return;

  RejectAllTimeoutCalls();

  auto op = operators_.EraseFree();
  if (op) {
    CallPtr call = calls_.PopFromQueue();
    if (call) {
      StartCallProcessing(call, op);
    } else {
      operators_.InsertFree(op);
    }
  } else {
    CallPtr call = calls_.GetMinTimeoutCallInQueue();
    if (call) {
      ScheduleCallProcessingIteration(call->GetTimeoutPoint());
    }
  }
}

/**
 * Start processing the call.
 * @param call - next call from the queue;
 * @param op - free operator who will handle the call.
 */
void CallCenter::StartCallProcessing(const CallPtr &call,
                                     const OperatorPtr &op) {
  logger_.Debug() << "Start call (" << boost::uuids::to_string(call->GetId()) << ") processing";
  call->StartProcessing();
  call->SetOperatorId(op->GetId());
  op->HandleCall(call,
                 [call, op, call_center = shared_from_this()]() {
                   call_center->FinishCallProcessing(call, op);
                 });
}

/**
 * Start processing a call if the queue is empty, there is a free operator and the call is unique.
 * It is necessary to bypass the queue, as it may have zero capacity.
 * @param call - call for processing.
 * @return True if processing has started, otherwise - false.
 */
bool CallCenter::StartCallProcessingIfPossible(const CallPtr &call) {
  if (!calls_.QueueIsEmpty())
    return false;

  auto op = operators_.EraseFree();
  if (op) {
    const auto added = calls_.InsertToProcessing(call);
    if (added) {
      StartCallProcessing(call, op);
      return true;
    } else {
      operators_.InsertFree(op);
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
void CallCenter::FinishCallProcessing(const CallPtr &call,
                                      const OperatorPtr &op) {
  logger_.Debug() << "Finish call processing (" << boost::uuids::to_string(call->GetId()) << ")";
  call->SetStatus(CallStatus::kOk);
  call->FinishProcessing();
  calls_.EraseFromProcessing(call);
  operators_.InsertFree(op);
  journal_.AddRecord(*call);

  // now there is at least one free operator
  if (!calls_.QueueIsEmpty()) {
    task_manager_.PostTask<void()>([call_center = shared_from_this()]() {
      call_center->PerformCallProcessingIteration();
    });
  }
}

/**
 * Schedule a new iteration of call processing at a certain time point.
 * @param time_point - time of execution of the new iteration of call processing.
 */
void CallCenter::ScheduleCallProcessingIteration(const CallDetailedRecord::TimePoint &time_point) {
  logger_.Debug() << "Add task to call processing at: " << time_point;

  const auto task = [call_center = shared_from_this()]() {
    call_center->PerformCallProcessingIteration();
  };
  task_manager_.PostTaskAt<void()>(time_point, task);
}

/**
 * Cancel call with for the specified reason.
 * @param call - call to reject;
 * @param reason - reason for rejection.
 */
void CallCenter::RejectCall(const CallPtr &call, CallStatus reason) {
  logger_.Info() << "Reject call (" << boost::uuids::to_string(call->GetId())
                 << ") - " << reason;
  call->SetStatus(reason);
  call->FinishProcessing();
  journal_.AddRecord(*call);
}

/**
 * Cancel all calls with expired waiting time.
 */
void CallCenter::RejectAllTimeoutCalls() {
  auto to_reject = calls_.EraseTimeoutCallFromQueue();
  while (to_reject) {
    RejectCall(to_reject, CallStatus::kTimeout);
    to_reject = calls_.EraseTimeoutCallFromQueue();
  }
}

} // call_center