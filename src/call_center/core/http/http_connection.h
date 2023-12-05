#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_CONNECTION_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_CONNECTION_H_

#include <atomic>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/config.hpp>
#include <unordered_map>

#include "http.h"
#include "http_repository.h"
#include "log/logger.h"
#include "log/logger_provider.h"

namespace call_center::core::http {

/**
 * @brief HTTP-соединение по заданному уже открытому сокету.
 */
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 public:
  /**
   * @brief Создать HTTP-соединение.
   * @param socket открытый сокет для HTTP-соединения
   * @param repositories множество HTTP-репозиториев для перенаправления запроса
   * @param logger_provider провайдер логера
   */
  static std::shared_ptr<HttpConnection> Create(
      tcp::socket &&socket,
      const std::unordered_map<std::string_view, std::shared_ptr<HttpRepository>> &repositories,
      const log::LoggerProvider &logger_provider
  );

  HttpConnection(const HttpConnection &other) = delete;
  HttpConnection &operator=(const HttpConnection &other) = delete;

  /**
   * @brief Запустить асинхронное чтение запроса,
   * при завершении вызовется метод @link OnReadRequest @endlink.
   */
  void ReadRequest();

 private:
  HttpConnection(
      tcp::socket &&socket,
      const std::unordered_map<std::string_view, std::shared_ptr<HttpRepository>> &repositories,
      const log::LoggerProvider &logger_provider
  );

  /**
   * @brief Следующий идентификатор соединения. Используется для указания тега при логировании.
   */
  static std::atomic_size_t next_id_;

  const std::unique_ptr<log::Logger> logger_;
  beast::tcp_stream stream_;
  /**
   * @brief Временный буфер, используемый при чтении запроса.
   */
  beast::flat_buffer buffer_;
  /**
   * @brief Множество путей отображенных на репозитории, которым будет перенаправляться запрос.
   */
  const std::unordered_map<std::string_view, std::shared_ptr<HttpRepository>> &repositories_;

  /**
   * @brief Сформировать HTTP-ответ "Not found".
   */
  static HttpRepository::Response MakeNotFoundResponse();

  void WriteResponse(HttpRepository::Response &&response);
  void OnWriteResponse(bool keep_alive, const beast::error_code &error_code);
  void Close();
  void OnReadRequest(const HttpRepository::Request &request, const beast::error_code &ec);
};

}  // namespace call_center::core::http

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_CONNECTION_H_
