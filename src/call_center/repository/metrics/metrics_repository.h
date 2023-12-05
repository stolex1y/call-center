#ifndef METRICS_REPOSITORY_H
#define METRICS_REPOSITORY_H

#include "core/http/http.h"
#include "core/http/http_repository.h"
#include "core/queueing_system/metrics/queueing_system_metrics.h"

namespace call_center::repository {

namespace qs = core::qs;
namespace b_http = core::http::http;
using namespace core::http;

/**
 * @brief HTTP-репозиторий для обработки запросов, связанных с метриками.
 */
class MetricsRepository : public HttpRepository,
                          public std::enable_shared_from_this<MetricsRepository> {
 public:
  static std::shared_ptr<MetricsRepository> Create(
      std::shared_ptr<const qs::metrics::QueueingSystemMetrics> metrics,
      const log::LoggerProvider &logger_provider
  );

  MetricsRepository(const MetricsRepository &other) = delete;
  MetricsRepository &operator=(const MetricsRepository &other) = delete;

  void HandleRequest(const b_http::request<b_http::string_body> &request, const OnHandle &on_handle)
      override;

 private:
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<const qs::metrics::QueueingSystemMetrics> metrics_;

  /**
   * @brief Сформировать тело ответа на запрос о получении метрик.
   */
  std::string MakeGetMetricsResponseBody() const;

  MetricsRepository(
      std::shared_ptr<const qs::metrics::QueueingSystemMetrics> metrics,
      const log::LoggerProvider &logger_provider
  );
};

}  // namespace call_center::repository

#endif  // METRICS_REPOSITORY_H
