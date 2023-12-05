#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_REPOSITORY_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_REPOSITORY_H_

#include "http.h"

namespace call_center::core::http {

/**
 * @brief Базовый класс для HTTP-репозиториев.
 *
 * Базовый класс для HTTP-репозиториев, классов,
 * которые обрабатывают запросы по заданному @link GetRootPath корню@endlink.
 */
class HttpRepository {
 public:
  /**
   * @brief Обратный вызов при завершении обработки запроса.
   */
  using OnHandle = std::function<void(http::response<http::string_body> &&)>;
  /**
   * @brief Принимаемый запрос.
   */
  using Request = http::request<http::string_body>;
  /**
   * @brief Сформированный ответ репозитория.
   */
  using Response = http::response<http::string_body>;

  explicit HttpRepository(std::string root);
  HttpRepository(const HttpRepository &other) = delete;
  HttpRepository &operator=(const HttpRepository &other) = delete;
  virtual ~HttpRepository() = default;

  /**
   * @brief Обработка входящих запросов.
   * @param on_handle обратный вызов при завершении обработки запроса
   */
  virtual void HandleRequest(
      const http::request<http::string_body> &request, const OnHandle &on_handle
  ) = 0;
  /**
   * @brief Корень запросов, обрабатываемых резиторием.
   */
  [[nodiscard]] std::string_view GetRootPath() const;

  /**
   * @brief Сформировать ответ
   * @param status статус ответа
   * @param keep_alive поддерживать соединение
   * @param body тело ответа
   */
  virtual Response MakeResponse(http::status status, bool keep_alive, std::string &&body);

 private:
  /**
   * @brief Корень для всех запросов, обрабатываемых репозиторием.
   */
  std::string root_;
};

}  // namespace call_center::core::http

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_REPOSITORY_H_
