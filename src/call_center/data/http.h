#ifndef CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_
#define CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_

#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace call_center::data {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

}

#endif //CALL_CENTER_SRC_CALL_CENTER_DATA_HTTP_H_
