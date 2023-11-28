#pragma once

#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast.hpp>

#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/version.hpp>
#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>
#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>
#include <openssl/ssl.h>
#include <openssl/hmac.h>
#include <boost/beast/ssl.hpp>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

enum class SideType {
    Buy,
    Sell,
};

enum class TypeOrder {
    LIMIT, 
    MARKET, 
    STOP_LOSS,
};

class SpotClient {
public:

    explicit SpotClient(std::string base_url);
    SpotClient(std::string base_url, std::string api_key, std::string secret_key);

    std::string ping();
    std::string time();
    std::string exchangeInfo(std::string symbol, std::vector<std::string> symbols);


    std::string account();

    std::string depth(std::string symbol);
    std::string trades(std::string symbol);
    std::string historicalTrades(std::string symbol);

    std::string klines(std::string symbol, std::string interval);
    std::string openOrders(std::string symbol);
    std::string allOrders(std::string symbol);

    std::string create_new_order_test(std::string symbol, SideType side, TypeOrder type, double quantity);

    std::string create_new_order(std::string symbol, SideType side, TypeOrder type, double quantity); 
    std::string info_about_order(std::string symbol, std::size_t order_id); 
    std::string cancel_order(std::string symbol, std::size_t order_id);

    std::string price(std::string symbol);
    std::string bookTicker(std::string symbol);

private:
    std::string Response(http::verb type, std::string sreq) {
        http::request<http::empty_body> req{type, sreq, 11};
        req.set(http::field::host, base_url_);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::write(stream_, req);
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream_, buffer, res);
        return beast::buffers_to_string(res.body().data());
    }

    void InitSession();

    static std::string GenSide(SideType side);
    static std::string GenType(TypeOrder type); 

    net::io_context ioc_;
    ssl::context ctx_;
    net::ip::tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;

    std::string base_url_;
    std::string api_key_;
    std::string secret_key_;
};
