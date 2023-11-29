#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_CALL_DETAILED_RECORD_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_CALL_DETAILED_RECORD_H_

#include "call_detailed_record.h"
#include "core/clock_adapter.h"

namespace call_center {

class FakeCallDetailedRecord : public CallDetailedRecord {
 public:
  static std::shared_ptr<FakeCallDetailedRecord> Create(
      std::shared_ptr<const core::ClockAdapter> clock,
      std::string caller_phone_number,
      std::shared_ptr<Configuration> configuration,
      OnFinish on_finish
  );

  FakeCallDetailedRecord(
      std::shared_ptr<const core::ClockAdapter> clock,
      std::string caller_phone_number,
      std::shared_ptr<Configuration> configuration,
      OnFinish on_finish
  );

  void SetArrivalTime() override;
  void StartService(boost::uuids::uuid operator_id) override;
  void CompleteService(CallStatus status) override;
  bool IsTimeout() const override;

 private:
  std::shared_ptr<const core::ClockAdapter> clock_;
};
}  // namespace call_center

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_CALL_DETAILED_RECORD_H_
