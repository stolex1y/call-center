#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_QUERY_REPOSITORY_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_QUERY_REPOSITORY_H_

#include <boost/utility/base_from_member.hpp>

#include "call_center.h"
#include "call_detailed_record.h"
#include "call_request_dto.h"
#include "http_repository.h"
#include "log/sink.h"

namespace call_center::data {

class CallRepository : private boost::base_from_member<std::unique_ptr<log::Logger>>,
                       public HttpRepository,
                       public std::enable_shared_from_this<CallRepository> {
 public:
  static std::shared_ptr<CallRepository> Create(
      std::shared_ptr<CallCenter> call_center,
      std::shared_ptr<Configuration> configuration,
      const std::shared_ptr<const log::LoggerProvider> &logger_provider
  );

  CallRepository(const CallRepository &other) = delete;
  CallRepository &operator=(const CallRepository &other) = delete;

  void HandleRequest(const http::request<http::string_body> &request, const OnHandle &on_handle)
      override;

 private:
  using logger_t = boost::base_from_member<std::unique_ptr<log::Logger>>;

  const std::shared_ptr<CallCenter> call_center_;
  const std::shared_ptr<Configuration> configuration_;

  static std::string MakeResponseBody(const CallDetailedRecord &cdr);

  CallRepository(
      std::shared_ptr<CallCenter> call_center,
      std::shared_ptr<Configuration> configuration,
      std::unique_ptr<log::Logger> logger
  );

  std::optional<CallRequestDto> ParseRequestBody(const std::string_view &body);
};

}  // namespace call_center::data

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_QUERY_REPOSITORY_H_
