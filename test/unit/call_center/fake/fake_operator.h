#ifndef CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_OPERATOR_H_
#define CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_OPERATOR_H_

#include <atomic>

#include "operator.h"

namespace call_center::test {

class FakeOperator : public Operator {
 public:
  static std::shared_ptr<FakeOperator> Create(
      std::shared_ptr<Configuration> configuration, const log::LoggerProvider &logger_provider
  );

  void HandleCall(const std::shared_ptr<CallDetailedRecord> &call, const OnFinishHandle &on_finish)
      override;

 private:
  FakeOperator(
      std::shared_ptr<Configuration> configuration, const log::LoggerProvider &logger_provider
  );
};

}  // namespace call_center::test

#endif  // CALL_CENTER_TEST_UNIT_CALL_CENTER_FAKE_FAKE_OPERATOR_H_
