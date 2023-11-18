#include "http_server.h"

#include <stdexcept>

#include "http_connection.h"

namespace call_center::data {

HttpServer::HttpServer(boost::asio::io_context &ioc,
                       const boost::asio::ip::tcp::endpoint &endpoint,
                       log::Sink &sink)
    : ioc_(ioc),
      endpoint_(endpoint),
      acceptor_(ioc),
      logger_("HttpServer", sink),
      sink_(sink) {
  Open();
}

void HttpServer::Open() {
  beast::error_code error;
  boost::system::error_code system_error;

  // Open the acceptor
  system_error = acceptor_.open(endpoint_.protocol(), error);
  boost::ignore_unused(system_error);
  if (error) {
    throw std::runtime_error("Failed on open acceptor with error: " + error.message());
  }

  // Allow address reuse
  system_error = acceptor_.set_option(net::socket_base::reuse_address(true), error);
  boost::ignore_unused(system_error);
  if (error) {
    throw std::runtime_error("Failed on set reuse option with error: " + error.message());
  }

  // Bind to the server address
  system_error = acceptor_.bind(endpoint_, error);
  boost::ignore_unused(system_error);
  if (error) {
    throw std::runtime_error("Failed on bind with error: " + error.message());
  }

  // Start listening for connections
  system_error = acceptor_.listen(
      net::socket_base::max_listen_connections, error);
  boost::ignore_unused(system_error);
  if (error) {
    throw std::runtime_error("Failed on listen with error: " + error.message());
  }

  stopped_.store(false);
  logger_.Debug() << "Start listening for connections on " << std::to_string(endpoint_.port());
}

void HttpServer::Start() {
  if (!acceptor_.is_open()) {
    Open();
  }
  acceptor_.async_accept(ioc_,
                         [this](beast::error_code ec, tcp::socket socket) {
                           this->OnAccept(ec, std::move(socket));
                         });
}

void HttpServer::Stop() {
  stopped_.store(true);
  acceptor_.close();
  logger_.Info() << "Server stopped";
}

void HttpServer::OnAccept(beast::error_code error, tcp::socket socket) {
  if (error || stopped_) {
    if (error)
      logger_.Error() << "Failed on accept with error: " << error.message();
    else
      logger_.Debug() << "New connection reject";
    Stop();
  } else {
    HttpConnection::Create(std::move(socket), repositories_, sink_)->ReadRequest();
    Start();
  }
}

std::shared_ptr<HttpServer> HttpServer::Create(net::io_context &ioc, const tcp::endpoint &endpoint, log::Sink &sink) {
  return std::shared_ptr<HttpServer>(new HttpServer(ioc, endpoint, sink));
}

void HttpServer::AddRepository(const std::shared_ptr<HttpRepository> &repository) {
  repositories_.emplace(repository->GetRootPath(), repository);
}

} // data