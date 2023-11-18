#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_REPOSITORY_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_REPOSITORY_H_

#include "http.h"
#include "log/logger.h"

namespace call_center::data {

class HttpRepository {
 public:
  using OnHandle = std::function<void(http::response<http::string_body> &&)>;
  using Request = http::request<http::string_body>;
  using Response = http::response<http::string_body>;

  HttpRepository(std::string root, log::Logger &logger);
  virtual ~HttpRepository() = default;

  virtual void HandleRequest(const http::request<http::string_body> &request, const OnHandle &on_handle) = 0;
  [[nodiscard]] std::string_view GetRootPath() const;

 protected:
  log::Logger &logger_;

  virtual Response MakeResponse(http::status status, bool keep_alive, std::string &&body);

 private:
  std::string root_;

};

} // data

#endif //CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_REPOSITORY_H_
