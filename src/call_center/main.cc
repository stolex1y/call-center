#include "data/http_server.h"
#include "data/call_repository.h"
#include "call_center.h"
#include "configuration.h"
#include "journal.h"
#include "core/task_manager.h"

using namespace call_center;
using namespace call_center::data;

int main() {
  log::Sink sink(log::SeverityLevel::kTrace);

  auto const port = static_cast<unsigned short>(8080);
  auto const address = net::ip::address_v4::any();

  Configuration configuration;
  core::TaskManager task_manager(sink);
  std::shared_ptr<CallCenter> call_center = CallCenter::Create(configuration, task_manager, sink);
  std::shared_ptr<HttpServer> http_server = HttpServer::Create(task_manager.IoContext(),
                                                               tcp::endpoint{address, port},
                                                               sink);
  std::shared_ptr<CallRepository> call_repository = CallRepository::Create(call_center, configuration, sink);
  http_server->AddRepository(call_repository);
  http_server->Start();
  task_manager.Start();
  task_manager.Join();
}
