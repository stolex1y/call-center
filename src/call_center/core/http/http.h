#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>

namespace call_center::core::http {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

}  // namespace call_center::core::http

#endif  // CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_
