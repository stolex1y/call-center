#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <functional>

#include "call_status.h"
#include "configuration.h"
#include "queueing_system/request.h"

namespace call_center {

class CallDetailedRecord : public qs::Request {
public:
  using OnFinish = std::function<void(const CallDetailedRecord &cdr)>;
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using WaitingDuration = std::chrono::seconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  static constexpr auto kMaxWaitKey_ = "call_max_wait";

  CallDetailedRecord(
      std::string caller_phone_number,
      std::shared_ptr<Configuration> configuration,
      OnFinish on_finish
  );
  ~CallDetailedRecord() override = default;

  [[nodiscard]] std::optional<Duration> GetServiceTime() const override;
  [[nodiscard]] std::optional<Duration> GetTotalTime() const override;
  [[nodiscard]] std::optional<TimePoint> GetArrivalTime() const override;
  [[nodiscard]] std::optional<TimePoint> GetServiceCompleteTime() const override;
  [[nodiscard]] std::optional<TimePoint> GetServiceStartTime() const override;
  [[nodiscard]] std::optional<Duration> GetWaitTime() const override;
  [[nodiscard]] bool WasFinished() const override;
  [[nodiscard]] bool WasServiced() const override;
  [[nodiscard]] bool WasArrived() const override;
  [[nodiscard]] bool IsTimeout() const override;

  virtual void SetArrivalTime();
  virtual void StartService(boost::uuids::uuid operator_id);
  virtual void CompleteService(CallStatus status);
  [[nodiscard]] virtual const std::string &GetCallerPhoneNumber() const;
  [[nodiscard]] virtual std::optional<CallStatus> GetStatus() const;
  [[nodiscard]] virtual std::optional<boost::uuids::uuid> GetOperatorId() const;
  [[nodiscard]] virtual WaitingDuration GetMaxWait() const;
  [[nodiscard]] virtual std::optional<TimePoint> GetTimeoutPoint() const;

 protected:
  static constexpr WaitingDuration kDefaultMaxWait_{30};

  mutable std::shared_mutex mutex_;
  const std::shared_ptr<Configuration> configuration_;
  std::optional<TimePoint> arrival_time_;
  std::optional<TimePoint> complete_service_time_;
  std::optional<TimePoint> start_service_time_;
  WaitingDuration max_wait_ = kDefaultMaxWait_;
  std::optional<TimePoint> timeout_point_;
  const std::string caller_phone_number_;
  std::optional<CallStatus> status_ = std::nullopt;
  std::optional<boost::uuids::uuid> operator_id_ = std::nullopt;
  const OnFinish on_finish_;

  [[nodiscard]] uint64_t ReadMaxWait() const;
  [[nodiscard]] bool WasFinished_() const;
  [[nodiscard]] bool WasServiced_() const;
  [[nodiscard]] bool WasArrived_() const;
};

}  // namespace call_center

#endif  // CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
