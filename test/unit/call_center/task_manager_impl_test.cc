#include "core/task_manager_impl.h"

#include <gtest/gtest.h>

#include "configuration_adapter.h"

namespace call_center::core::test {

using namespace log;
using namespace std::chrono_literals;
using namespace std::chrono;

class TaskManagerImplTest : public testing::Test {
 public:
  TaskManagerImplTest();

  const std::string test_name_;
  const std::shared_ptr<const LoggerProvider> logger_provider_;
  const std::shared_ptr<Configuration> configuration_;
  ConfigurationAdapter configuration_adapter_;
  const std::shared_ptr<TaskManagerImpl> task_manager_;
};

TaskManagerImplTest::TaskManagerImplTest()
    : test_name_(testing::UnitTest::GetInstance()->current_test_info()->name()),
      logger_provider_(std::make_shared<LoggerProvider>(
          std::make_unique<Sink>(test_name_ + ".log", SeverityLevel::kTrace, SIZE_MAX)
      )),
      configuration_(Configuration::Create(logger_provider_, "config-" + test_name_ + ".json")),
      configuration_adapter_(configuration_),
      task_manager_(TaskManagerImpl::Create(configuration_, logger_provider_)) {
  configuration_adapter_.SetConfigurationCaching(false);
  configuration_adapter_.UpdateConfiguration();
}

}  // namespace call_center::core::test
