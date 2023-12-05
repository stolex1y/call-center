#include "call_center.h"
#include "configuration.h"
#include "configuration_updater.h"
#include "core/http/http_server.h"
#include "core/tasks/task_manager_impl.h"
#include "journal.h"
#include "repository/call/call_repository.h"
#include "repository/metrics/metrics_repository.h"

using namespace call_center;
using namespace call_center::core::http;
using namespace call_center::log;
using namespace call_center::core;
using namespace call_center::core::tasks;
using namespace call_center::core::qs::metrics;
using namespace call_center::repository;

int main() {
  const LoggerProvider logger_provider(std::make_shared<Sink>(SeverityLevel::kTrace));

  constexpr auto port = static_cast<unsigned short>(8080);
  const auto address = net::ip::address_v4::any();

  const auto configuration = Configuration::Create(logger_provider);
  auto task_manager = TaskManagerImpl::Create(configuration, logger_provider);
  ConfigurationUpdater::Create(configuration, task_manager, logger_provider)->StartUpdating();
  const auto operator_provider = [&task_manager, &configuration, &logger_provider] {
    return Operator::Create(task_manager, configuration, logger_provider);
  };
  const auto metrics = QueueingSystemMetrics::Create(task_manager, configuration, logger_provider);
  const auto call_center = CallCenter::Create(
      std::make_unique<Journal>(configuration),
      configuration,
      task_manager,
      logger_provider,
      std::make_unique<OperatorSet>(configuration, operator_provider, logger_provider, metrics),
      std::make_unique<CallQueue>(configuration, logger_provider),
      metrics
  );
  const auto http_server =
      HttpServer::Create(task_manager->IoContext(), tcp::endpoint{address, port}, logger_provider);
  http_server->AddRepository(CallRepository::Create(call_center, configuration, logger_provider));
  http_server->AddRepository(MetricsRepository::Create(metrics, logger_provider));
  task_manager->Start();
  http_server->Start();
  task_manager->Join();
}
