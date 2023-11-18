#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_CONNECTION_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_CONNECTION_H_

#include <unordered_map>
#include <string>
#include <atomic>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/beast/http/status.hpp>

#include "http.h"
#include "log/logger.h"
#include "log/sink.h"
#include "http_repository.h"

namespace call_center::data {

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 public:
  static std::shared_ptr<HttpConnection> Create(tcp::socket &&socket,
                                                const std::unordered_map<
                                                    std::string_view,
                                                    std::shared_ptr<HttpRepository>> &repositories,
                                                log::Sink &sink);

  HttpConnection(const HttpConnection &other) = delete;
  HttpConnection &operator=(const HttpConnection &other) = delete;

  void ReadRequest();

 private:
  HttpConnection(tcp::socket &&socket,
                 const std::unordered_map<std::string_view, std::shared_ptr<HttpRepository>> &repositories,
                 log::Sink &sink);

  static std::atomic_size_t next_id_;

  const size_t id_ = next_id_.fetch_add(1);
  log::Logger logger_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  const std::unordered_map<std::string_view, std::shared_ptr<HttpRepository>> &repositories_;

  static HttpRepository::Response MakeNotFoundResponse();

  void WriteResponse(HttpRepository::Response &&response);
  void OnWriteResponse(bool keep_alive, beast::error_code error_code);
  void Close();
  void OnReadRequest(const HttpRepository::Request &request, beast::error_code ec);
};

} // data

#endif //CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_CONNECTION_H_
