#include "http_connection.h"

#include <chrono>

using namespace std::chrono_literals;

namespace call_center::data {

std::atomic_size_t HttpConnection::next_id_ = 0;

HttpConnection::HttpConnection(tcp::socket &&socket,
                               const std::unordered_map<std::string_view,
                                                        std::shared_ptr<HttpRepository>> &repositories,
                               log::Sink &sink)
    : logger_("HttpConnection (" + std::to_string(id_) + ")", sink),
      stream_(std::move(socket)), repositories_(repositories) {
  stream_.expires_after(30s);
}

void HttpConnection::ReadRequest() {
  auto request = std::make_shared<HttpRepository::Request>();

  http::async_read(stream_, buffer_, *request,
                   [request, conn = shared_from_this()](beast::error_code ec,
                                                        std::size_t bytes_transferred) {
                     boost::ignore_unused(bytes_transferred);
                     conn->OnReadRequest(*request, ec);
                   });
}

void HttpConnection::OnReadRequest(const HttpRepository::Request &request, beast::error_code ec) {
  if (ec == http::error::end_of_stream) {
    Close();
    return;
  }

  if (ec) {
    Close();
    logger_.Error() << "Failed on read request: " << ec.what();
    return;
  }

  logger_.Info() << "Read request: " << to_string(request.method()) << " " << request.target();

  auto path_root = request.target().substr(1, request.target().find_first_of("/", 1));
  auto repository = repositories_.find(path_root);
  if (repository == repositories_.end()) {
    logger_.Info() << "No processing repository found";
    WriteResponse(MakeNotFoundResponse());
  } else {
    logger_.Info() << "Redirect request to repository";
    repository->second->HandleRequest(
        request,
        [conn = shared_from_this()](HttpRepository::Response &&response) {
          conn->WriteResponse(std::move(response));
        });
  }
}

void HttpConnection::WriteResponse(HttpRepository::Response &&response) {
  logger_.Info() << "Write response: " << response.result();
  bool keep_alive = response.keep_alive();
  beast::async_write(stream_,
                     http::message_generator{std::move(response)},
                     [conn = shared_from_this(), keep_alive](beast::error_code ec,
                                                             std::size_t bytes_transferred) {
                       boost::ignore_unused(bytes_transferred);
                       conn->OnWriteResponse(keep_alive, ec);
                     });
}

void HttpConnection::OnWriteResponse(bool keep_alive, beast::error_code error_code) {
  if (error_code) {
    Close();
    logger_.Error() << "Failed on write response: " << error_code.what();
  }

  if (keep_alive) {
    ReadRequest();
  } else {
    Close();
  }
}

void HttpConnection::Close() {
  beast::error_code error;
  logger_.Info() << "Close connection";
  boost::system::error_code system_error = stream_.socket().shutdown(tcp::socket::shutdown_both, error);
  boost::ignore_unused(system_error);
}

std::shared_ptr<HttpConnection> HttpConnection::Create(tcp::socket &&socket,
                                                       const std::unordered_map<
                                                           std::string_view,
                                                           std::shared_ptr<HttpRepository>> &repositories,
                                                       log::Sink &sink) {
  return std::shared_ptr<HttpConnection>(new HttpConnection(std::move(socket), repositories, sink));
}

HttpRepository::Response HttpConnection::MakeNotFoundResponse() {
  HttpRepository::Response response{};
  response.result(http::status::not_found);
  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.keep_alive(false);
  return response;
}

} // data