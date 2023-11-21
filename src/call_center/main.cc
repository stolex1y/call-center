#include "call_center.h"
#include "configuration.h"
#include "configuration_updater.h"
#include "core/task_manager.h"
#include "data/call_repository.h"
#include "data/http_server.h"
#include "journal.h"

using namespace call_center;
using namespace call_center::data;
using namespace call_center::log;
using namespace call_center::core;

int main() {
  const auto logger_provider = std::make_shared<LoggerProvider>(
      std::make_unique<Sink>(SeverityLevel::kTrace)
  );

  const auto port = static_cast<unsigned short>(8080);
  const auto address = net::ip::address_v4::any();

  const auto configuration = Configuration::Create(logger_provider);
  auto task_manager =
      std::make_shared<TaskManager>(configuration, logger_provider);
  ConfigurationUpdater::Create(configuration, task_manager, logger_provider)
      ->StartUpdating();
  auto call_center = CallCenter::Create(
      std::make_unique<Journal>(configuration),
      configuration,
      task_manager,
      logger_provider,
      std::make_unique<OperatorSet>(
          configuration, task_manager, logger_provider
      ),
      std::make_unique<CallQueue>(configuration, logger_provider)
  );
  auto http_server = HttpServer::Create(
      task_manager->IoContext(), tcp::endpoint{address, port}, logger_provider
  );
  http_server->AddRepository(
      CallRepository::Create(call_center, configuration, logger_provider)
  );
  task_manager->Start();
  http_server->Start();
  task_manager->Join();
}
