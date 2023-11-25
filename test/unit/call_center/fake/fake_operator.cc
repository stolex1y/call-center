#include "fake_operator.h"

namespace call_center::test {

std::shared_ptr<FakeOperator> FakeOperator::Create(
    std::shared_ptr<Configuration> configuration, const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<FakeOperator>(new FakeOperator(std::move(configuration), logger_provider));
}

FakeOperator::FakeOperator(
    std::shared_ptr<Configuration> configuration, const log::LoggerProvider &logger_provider
)
    : Operator(nullptr, std::move(configuration), logger_provider) {
}

boost::uuids::uuid FakeOperator::GetId() const {
  return id_;
}

void FakeOperator::HandleCall(
    const std::shared_ptr<CallDetailedRecord> &call, const Operator::OnFinishHandle &on_finish
) {
  // Empty
}

}  // namespace call_center::test
