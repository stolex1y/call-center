#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_QUERY_REPOSITORY_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_QUERY_REPOSITORY_H_

#include "call_center.h"
#include "call_detailed_record.h"
#include "call_request_dto.h"
#include "core/http/http.h"
#include "core/http/http_repository.h"

/// Реализации HTTP-репозиториев.
namespace call_center::repository {

namespace b_http = http::http;

/**
 * @brief Репозиторий для обработки вызовов.
 */
class CallRepository : public http::HttpRepository,
                       public std::enable_shared_from_this<CallRepository> {
 public:
  static std::shared_ptr<CallRepository> Create(
      std::shared_ptr<CallCenter> call_center,
      std::shared_ptr<config::Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );

  CallRepository(const CallRepository &other) = delete;
  CallRepository &operator=(const CallRepository &other) = delete;

  void HandleRequest(const b_http::request<b_http::string_body> &request, const OnHandle &on_handle)
      override;

 private:
  const std::unique_ptr<log::Logger> logger_;
  const std::shared_ptr<CallCenter> call_center_;
  const std::shared_ptr<config::Configuration> configuration_;

  /**
   * @brief Сформировать ответ из обработанного вызова.
   */
  static std::string MakeResponseBody(const CallDetailedRecord &cdr);

  CallRepository(
      std::shared_ptr<CallCenter> call_center,
      std::shared_ptr<config::Configuration> configuration,
      const log::LoggerProvider &logger_provider
  );

  /**
   * @brief Сформировать объект вызова из тела запроса.
   */
  std::optional<CallRequestDto> ParseRequestBody(const std::string_view &body) const;
};

}  // namespace call_center::repository

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_QUERY_REPOSITORY_H_
