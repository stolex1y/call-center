#include "call_repository.h"

#include <boost/json.hpp>
#include <chrono>

#include "call_request_dto.h"
#include "call_response_dto.h"

using namespace std::chrono_literals;
namespace json = boost::json;

namespace call_center::repository {

std::shared_ptr<CallRepository> CallRepository::Create(
    std::shared_ptr<CallCenter> call_center,
    std::shared_ptr<config::Configuration> configuration,
    const log::LoggerProvider &logger_provider
) {
  return std::shared_ptr<CallRepository>(
      new CallRepository(std::move(call_center), std::move(configuration), logger_provider)
  );
}

CallRepository::CallRepository(
    std::shared_ptr<CallCenter> call_center,
    std::shared_ptr<config::Configuration> configuration,
    const log::LoggerProvider &logger_provider
)
    : HttpRepository("call"),
      logger_(logger_provider.Get("CallRepository")),
      call_center_(std::move(call_center)),
      configuration_(std::move(configuration)) {
}

void CallRepository::HandleRequest(
    const b_http::request<b_http::string_body> &request, const OnHandle &on_handle
) {
  logger_->Info() << "Start handle request: " << to_string(request.method()) << " "
                  << request.target();
  if (request.method() != b_http::verb::post) {
    logger_->Info() << "Cannot handle request with illegal method (" << to_string(request.method())
                    << ")";
    on_handle(MakeResponse(b_http::status::method_not_allowed, false, {}));
    return;
  }

  auto dto = ParseRequestBody(request.body());
  if (!dto) {
    on_handle(MakeResponse(b_http::status::bad_request, false, {}));
    return;
  }
  auto on_call_processing_finish = [repo = shared_from_this(), on_handle](const auto &call) {
    auto response = repo->MakeResponse(b_http::status::ok, false, MakeResponseBody(call));
    on_handle(std::move(response));
  };
  const auto call = std::make_shared<CallDetailedRecord>(
      dto->phone, configuration_, std::move(on_call_processing_finish)
  );
  call_center_->PushCall(call);
}

std::optional<CallRequestDto> CallRepository::ParseRequestBody(const std::string_view &body) const {
  try {
    const auto json_body = json::parse(body);
    return json::value_to<CallRequestDto>(json_body);
  } catch ([[maybe_unused]] const json::system_error &error) {
    logger_->Info() << "Invalid request body: " << body;
    return std::nullopt;
  }
}

std::string CallRepository::MakeResponseBody(const CallDetailedRecord &cdr) {
  assert(cdr.WasFinished());
  auto &&call_response_dto = CallResponseDto(*cdr.GetStatus());
  return serialize(json::value_from(call_response_dto));
}

}  // namespace call_center::repository
