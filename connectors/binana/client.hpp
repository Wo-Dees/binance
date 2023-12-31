#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <binana/order.hpp>

#include <string>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

namespace binana {

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
    std::string exchangeInfo(const std::string& symbol);

    std::string depth(const std::string& symbol, std::size_t limit);
    std::string trades(const std::string& symbol, std::size_t limit);
    std::string klines(const std::string& symbol, const std::string& interval);
    std::string historicalTrades(const std::string& symbol);
    std::string price(const std::string& symbol);
    std::string bookTicker(const std::string& symbol);

    std::string account();
    std::string openOrders(const std::string& symbol);
    std::string allOrders(const std::string& symbol);

    std::string create_new_order_market_test(const std::string& symbol, SideType side, double quantity);
    std::string create_new_market_order(const std::string& symbol, SideType side, double quantity); 

    OrderHandle create_new_limit_order(const std::string& symbol, SideType side, const std::string& timeInForce, double price, double quantity);
    std::string info_about_order(const std::string& symbol, OrderHandle handle); 
    std::string cancel_order(const std::string& symbol, OrderHandle handle);

private:

    std::string Response(http::verb type, const std::string& sreq);

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

}
