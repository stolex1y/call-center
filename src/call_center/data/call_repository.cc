#include "call_repository.h"

#include <chrono>
#include <boost/json.hpp>

#include "call_request_dto.h"
#include "call_response_dto.h"

using namespace std::chrono_literals;
namespace json = boost::json;

namespace call_center::data {

std::shared_ptr<CallRepository> CallRepository::Create(std::shared_ptr<CallCenter> call_center,
                                                       std::shared_ptr<const Configuration> configuration,
                                                       const std::shared_ptr<const log::LoggerProvider> &logger_provider) {
  return std::shared_ptr<CallRepository>(new CallRepository(std::move(call_center),
                                                            std::move(configuration),
                                                            logger_provider->Get("CallRepository")));
}

CallRepository::CallRepository(std::shared_ptr<CallCenter> call_center,
                               std::shared_ptr<const Configuration> configuration,
                               std::unique_ptr<log::Logger> logger)
    : logger_t(std::move(logger)),
      HttpRepository("call", *logger_t::member),
      call_center_(std::move(call_center)),
      configuration_(std::move(configuration)) {
}

void CallRepository::HandleRequest(const http::request<http::string_body> &request,
                                   const HttpRepository::OnHandle &on_handle) {
  logger_.Info() << "Start handle request: " << to_string(request.method()) << " " << request.target();
  if (request.method() != http::verb::post) {
    logger_.Info() << "Cannot handle request with illegal method ("
                   << to_string(request.method())
                   << ")";
    on_handle(MakeResponse(http::status::method_not_allowed, false, {}));
    return;
  }

  auto dto = ParseRequestBody(request.body());
  if (!dto) {
    on_handle(MakeResponse(http::status::bad_request, false, {}));
    return;
  } else {
    auto on_call_processing_finish =
        [repo = shared_from_this(), on_handle](const auto &call) {
          auto response = repo->MakeResponse(http::status::ok, false, MakeResponseBody(call));
          on_handle(std::move(response));
        };
    auto call = std::make_shared<CallDetailedRecord>(
        dto->phone, configuration_,
        std::move(on_call_processing_finish));
    call_center_->PushCall(call);
  }
}

std::optional<CallRequestDto> CallRepository::ParseRequestBody(const std::string_view &body) {
  try {
    const auto json_body = json::parse(body);
    return json::value_to<CallRequestDto>(json_body);
  } catch (const json::system_error &error) {
    logger_.Info() << "Invalid request body: " << body;
    return std::nullopt;
  }
}

std::string CallRepository::MakeResponseBody(const CallDetailedRecord &cdr) {
  CallResponseDto &&call_response_dto{to_string(cdr.GetStatus())};
  return serialize(json::value_from(call_response_dto));
}

}
