#include "metrics_repository.h"

#include "metrics_response_dto.h"

using namespace std::chrono_literals;
namespace json = boost::json;

namespace call_center::repository {
std::shared_ptr<MetricsRepository> MetricsRepository::Create(
    std::shared_ptr<const QueueingSystemMetrics> metrics, const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<MetricsRepository>(
      new MetricsRepository(std::move(metrics), logger_provider)
  );
}

MetricsRepository::MetricsRepository(
    std::shared_ptr<const QueueingSystemMetrics> metrics, const log::LoggerProvider &logger_provider
)
    : HttpRepository("metrics"),
      logger_(logger_provider.Get("MetricsRepository")),
      metrics_(std::move(metrics)) {
}

void MetricsRepository::HandleRequest(
    const http::request<http::string_body> &request, const OnHandle &on_handle
) {
  logger_->Info() << "Start handle request: " << to_string(request.method()) << " "
                  << request.target();
  if (request.method() != http::verb::get) {
    logger_->Info() << "Cannot handle request with illegal method (" << to_string(request.method())
                    << ")";
    on_handle(MakeResponse(http::status::method_not_allowed, false, {}));
    return;
  }
  on_handle(MakeResponse(http::status::ok, false, MakeResponseBody()));
}

std::string MetricsRepository::MakeResponseBody() const {
  const MetricsReponseDto response_dto(
      metrics_->GetWaitTimeMetric(),
      metrics_->GetQueueSizeMetric(),
      metrics_->GetBusyServerCountMetric(),
      metrics_->GetServiceLoadInErlang()
  );
  return serialize(json::value_from(response_dto));
}

}  // namespace call_center::repository
