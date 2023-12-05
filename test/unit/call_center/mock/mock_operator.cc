#include "mock_operator.h"

namespace call_center::test {

std::shared_ptr<MockOperator> MockOperator::Create(
    std::shared_ptr<Configuration> configuration, const log::LoggerProvider& logger_provider
) {
  return std::shared_ptr<MockOperator>(new MockOperator(std::move(configuration), logger_provider));
}

MockOperator::MockOperator(
    std::shared_ptr<Configuration> configuration, const log::LoggerProvider& logger_provider
)
    : Operator(nullptr, std::move(configuration), logger_provider) {
}

void MockOperator::HandleCall(
    const std::shared_ptr<CallDetailedRecord>& call, const OnFinishHandle& on_finish
) {
  // Empty
}

}  // namespace call_center::test
