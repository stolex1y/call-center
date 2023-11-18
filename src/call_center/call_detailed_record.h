#ifndef CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
#define CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_

#include <chrono>

#include <boost/uuid/uuid.hpp>

#include <functional>

#include "call_status.h"
#include "configuration.h"

namespace call_center {

class CallDetailedRecord {
 public:
  using OnFinish = std::function<void(const CallDetailedRecord &cdr)>;
  using Clock = std::chrono::utc_clock;
  using Duration = std::chrono::milliseconds;
  using TimePoint = std::chrono::time_point<Clock, Duration>;

  CallDetailedRecord(std::string caller_phone_number,
                     const Configuration &configuration,
                     OnFinish on_finish);

  void StartProcessing();
  void FinishProcessing();
  [[nodiscard]] bool WasProcessed() const;
  void SetStatus(CallStatus status);
  void SetOperatorId(boost::uuids::uuid uuid);
  [[nodiscard]] std::optional<Duration> GetProcessingDuration() const;
  [[nodiscard]] const TimePoint &GetReceiptTime() const;
  [[nodiscard]] std::optional<TimePoint> GetEndProcessingTime() const;
  [[nodiscard]] std::optional<TimePoint> GetStartProcessingTime() const;
  [[nodiscard]] const boost::uuids::uuid &GetId() const;
  [[nodiscard]] const std::string &GetCallerPhoneNumber() const;
  [[nodiscard]] CallStatus GetStatus() const;
  [[nodiscard]] std::optional<boost::uuids::uuid> GetOperatorId() const;
  [[nodiscard]] const Duration &GetMaxWait() const;
  [[nodiscard]] bool IsTimeout() const;
  [[nodiscard]] const TimePoint &GetTimeoutPoint() const;
  [[nodiscard]] bool operator==(const CallDetailedRecord &other) const;

 private:
  static constexpr const auto kMaxWaitKey = "call_max_wait";
  static constexpr const uint64_t kDefaultMaxWait = 3;

  const Configuration &configuration_;
  const TimePoint receipt_time_;
  TimePoint end_processing_time_;
  TimePoint start_processing_time_;
  const Duration max_wait_;
  const TimePoint timeout_point_;
  const boost::uuids::uuid id_;
  const std::string caller_phone_number_;
  CallStatus status_ = CallStatus::kOk;
  std::optional<boost::uuids::uuid> operator_id_ = std::nullopt;
  const OnFinish on_finish_;

  [[nodiscard]] uint64_t ReadMaxWait() const;
};

} // call_center

#endif //CALL_CENTER_SRC_CALL_CENTER_CALL_DETAILED_RECORD_H_
