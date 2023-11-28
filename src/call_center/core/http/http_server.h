#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_SERVER_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_SERVER_H_

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <unordered_map>

#include "configuration.h"
#include "http.h"
#include "http_repository.h"
#include "log/logger.h"
#include "log/sink.h"

namespace call_center::core::http {
class HttpServer : public std::enable_shared_from_this<HttpServer> {
 public:
  HttpServer(const HttpServer &other) = delete;
  HttpServer &operator=(const HttpServer &other) = delete;

  static std::shared_ptr<HttpServer> Create(
      net::io_context &ioc, const tcp::endpoint &endpoint, log::LoggerProvider logger_provider
  );

  void AddRepository(const std::shared_ptr<HttpRepository> &repository);
  void Start();
  void Stop();

 private:
  std::unordered_map<std::string_view, std::shared_ptr<HttpRepository>> repositories_;
  net::io_context &ioc_;
  const tcp::endpoint &endpoint_;
  tcp::acceptor acceptor_;
  const log::LoggerProvider logger_provider_;
  const std::unique_ptr<log::Logger> logger_;
  std::atomic_bool stopped_ = true;

  HttpServer(
      net::io_context &ioc, const tcp::endpoint &endpoint, log::LoggerProvider logger_provider
  );

  void OnAccept(beast::error_code error, tcp::socket socket);
  void Open();
};

}  // namespace call_center::core::http

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_SERVER_H_
