#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <functional>

#include "call_status.h"
#include "configuration.h"

namespace call_center {

class CallDetailedRecord {
 public:
  using OnFinish = std::function<void(const CallDetailedRecord &cdr)>;
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using WaitingDuration = std::chrono::seconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  static constexpr const auto kMaxWaitKey_ = "call_max_wait";

  CallDetailedRecord(
      std::string caller_phone_number,
      std::shared_ptr<Configuration> configuration,
      OnFinish on_finish
  );

  virtual void StartProcessing(boost::uuids::uuid operator_id);
  virtual void FinishProcessing(CallStatus status);
  [[nodiscard]] virtual bool WasProcessed() const;
  [[nodiscard]] virtual std::optional<Duration> GetProcessingDuration() const;
  [[nodiscard]] virtual std::optional<Duration> GetTotalTime() const;
  [[nodiscard]] virtual TimePoint GetReceiptTime() const;
  [[nodiscard]] virtual std::optional<TimePoint> GetEndProcessingTime() const;
  [[nodiscard]] virtual std::optional<TimePoint> GetStartProcessingTime() const;
  [[nodiscard]] virtual boost::uuids::uuid GetId() const;
  [[nodiscard]] virtual const std::string &GetCallerPhoneNumber() const;
  [[nodiscard]] virtual std::optional<CallStatus> GetStatus() const;
  [[nodiscard]] virtual std::optional<boost::uuids::uuid> GetOperatorId() const;
  [[nodiscard]] virtual WaitingDuration GetMaxWait() const;
  [[nodiscard]] virtual bool IsTimeout() const;
  [[nodiscard]] virtual TimePoint GetTimeoutPoint() const;
  [[nodiscard]] virtual std::optional<Duration> GetWaitingDuration() const;
  [[nodiscard]] virtual bool WasFinished() const;
  [[nodiscard]] virtual bool operator==(const CallDetailedRecord &other) const;

 private:
  static constexpr const WaitingDuration kDefaultMaxWait_{30};

  mutable std::shared_mutex mutex_;
  const std::shared_ptr<Configuration> configuration_;
  const TimePoint receipt_time_;
  std::optional<TimePoint> end_processing_time_;
  std::optional<TimePoint> start_processing_time_;
  WaitingDuration max_wait_ = kDefaultMaxWait_;
  TimePoint timeout_point_;
  const boost::uuids::uuid id_;
  const std::string caller_phone_number_;
  std::optional<CallStatus> status_ = std::nullopt;
  std::optional<boost::uuids::uuid> operator_id_ = std::nullopt;
  const OnFinish on_finish_;

  [[nodiscard]] uint64_t ReadMaxWait() const;
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
