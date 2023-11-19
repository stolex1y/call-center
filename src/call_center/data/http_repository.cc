#include <boost/beast/version.hpp>
#include "http_repository.h"

namespace call_center::data {

HttpRepository::HttpRepository(std::string root, log::Logger &logger)
    : logger_(logger),
      root_(std::move(root)) {
}

std::string_view HttpRepository::GetRootPath() const {
  return root_;
}

HttpRepository::Response HttpRepository::MakeResponse(http::status status, bool keep_alive, std::string &&body) {
  HttpRepository::Response response{};
  response.result(status);
  response.set(http::field::content_type, "application/json");
  response.body() = std::move(body);
  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.keep_alive(keep_alive);
  response.prepare_payload();
  return response;
}

}
