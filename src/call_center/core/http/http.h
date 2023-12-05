#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_

/**
 * @file
 * @brief Заголовочный файл для более краткого доступа к библиотекам Boost для HTTP.
 */

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>

/**
 * @brief Содержит классы для HTTP-соединений.
 */
namespace call_center::core::http {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

}  // namespace call_center::core::http

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_
