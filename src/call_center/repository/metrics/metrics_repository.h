#ifndef METRICS_REPOSITORY_H
#define METRICS_REPOSITORY_H
#include "core/http/http_repository.h"
#include "queueing_system/metrics/queueing_system_metrics.h"

namespace call_center::repository {
using namespace core::http;

class MetricsRepository : public HttpRepository,
                          public std::enable_shared_from_this<MetricsRepository> {
 public:
  static std::shared_ptr<MetricsRepository> Create(
      std::shared_ptr<const qs::metrics::QueueingSystemMetrics> metrics,
      const log::LoggerProvider &logger_provider
  );

  MetricsRepository(const MetricsRepository &other) = delete;
  MetricsRepository &operator=(const MetricsRepository &other) = delete;

  void HandleRequest(const http::request<http::string_body> &request, const OnHandle &on_handle)
      override;

 private:
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<const qs::metrics::QueueingSystemMetrics> metrics_;

  std::string MakeResponseBody() const;

  MetricsRepository(
      std::shared_ptr<const qs::metrics::QueueingSystemMetrics> metrics,
      const log::LoggerProvider &logger_provider
  );
};

}  // namespace call_center::repository

#endif  // METRICS_REPOSITORY_H
