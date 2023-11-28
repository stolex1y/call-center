#include "http_repository.h"

#include <boost/beast/version.hpp>

namespace call_center::core::http {
HttpRepository::HttpRepository(std::string root) : root_(std::move(root)) {
}

std::string_view HttpRepository::GetRootPath() const {
  return root_;
}

HttpRepository::Response HttpRepository::MakeResponse(
    http::status status, bool keep_alive, std::string &&body
) {
  Response response{};
  response.result(status);
  response.set(http::field::content_type, "application/json");
  response.body() = std::move(body);
  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.keep_alive(keep_alive);
  response.prepare_payload();
  return response;
}

}  // namespace call_center::core::http
