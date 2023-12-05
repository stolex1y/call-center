#include "call_center.h"

#include <boost/uuid/uuid_io.hpp>
#include <chrono>

namespace call_center {

using namespace std::chrono_literals;
using namespace qs::metrics;

std::shared_ptr<CallCenter> CallCenter::Create(
    std::unique_ptr<Journal> journal,
    std::shared_ptr<Configuration> configuration,
    std::shared_ptr<tasks::TaskManager> task_manager,
    const log::LoggerProvider &logger_provider,
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
    std::shared_ptr<tasks::TaskManager> task_manager,
    const log::LoggerProvider &logger_provider,
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

void CallCenter::PushCall(const CallPtr &call) {
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

void CallCenter::PerformCallProcessingIteration() {
  if (calls_->QueueIsEmpty())
    return;

  RejectAllTimeoutCalls();

  const auto op = operators_->EraseFree();
  if (op) {
    const CallPtr call = calls_->PopFromQueue();
    if (call) {
      calls_->InsertToProcessing(call);
      StartCallProcessing(call, op);
    } else {
      operators_->InsertFree(op);
    }
  } else {
    const CallPtr call = calls_->GetMinTimeoutCallInQueue();
    if (call) {
      ScheduleCallProcessingIteration(*call->GetTimeoutPoint());
    }
  }
}

void CallCenter::StartCallProcessing(const CallPtr &call, const OperatorPtr &op) {
  logger_->Debug() << "Start call (" << boost::uuids::to_string(call->GetId()) << ") processing";
  call->StartService(op->GetId());
  metrics_->RecordServiceStart(call);
  op->HandleCall(call, [call, op, call_center = shared_from_this()]() {
    call_center->FinishCallProcessing(call, op);
  });
}

bool CallCenter::StartCallProcessingIfPossible(const CallPtr &call) {
  if (!calls_->QueueIsEmpty())
    return false;

  const auto op = operators_->EraseFree();
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

void CallCenter::FinishCallProcessing(const CallPtr &call, const OperatorPtr &op) {
  logger_->Debug() << "Finish call processing (" << boost::uuids::to_string(call->GetId()) << ")";
  call->CompleteService(CallStatus::kOk);
  metrics_->RecordServiceComplete(call, op);
  calls_->EraseFromProcessing(call);
  operators_->InsertFree(op);
  journal_->AddRecord(*call);

  // now there is at least one free operator
  if (!calls_->QueueIsEmpty()) {
    PerformCallProcessingIteration();
  }
}

void CallCenter::ScheduleCallProcessingIteration(const CallDetailedRecord::TimePoint &time_point) {
  logger_->Debug() << "Add task to call processing at: " << time_point;

  const auto task = [call_center = shared_from_this()]() {
    call_center->PerformCallProcessingIteration();
  };
  task_manager_->PostTaskAt(time_point, task);
}

void CallCenter::RejectCall(const CallPtr &call, const CallStatus reason) const {
  logger_->Info() << "Reject call (" << boost::uuids::to_string(call->GetId()) << ") - " << reason;
  call->CompleteService(reason);
  metrics_->RecordRequestDropout(call);
  journal_->AddRecord(*call);
}

void CallCenter::RejectAllTimeoutCalls() const {
  auto to_reject = calls_->EraseTimeoutCallFromQueue();
  while (to_reject) {
    RejectCall(to_reject, CallStatus::kTimeout);
    to_reject = calls_->EraseTimeoutCallFromQueue();
  }
}

}  // namespace call_center
