#include <order.hpp>
#include <client.hpp>
#include <hmac/hmac.hpp>

#include <boost/property_tree/json_parser.hpp>

namespace binana {

std::string getField(const std::string& json, const std::string& field) {
    std::stringstream jsonencode(json);
    boost::property_tree::ptree root;
    boost::property_tree::read_json(jsonencode, root);

    if (root.empty()) {
        return "";
    }
    return root.get<std::string>(field);
}

SpotClient::SpotClient(std::string base_url) : 
    ctx_(ssl::context::tlsv13_client), resolver_(ioc_), stream_(ioc_, ctx_),
    base_url_(std::move(base_url)) {
    InitSession();
}

SpotClient::SpotClient(std::string base_url, std::string api_key, std::string secret_key) : 
    ctx_(ssl::context::tlsv13_client), resolver_(ioc_), stream_(ioc_, ctx_),
    base_url_(std::move(base_url)), api_key_(std::move(api_key)), secret_key_(std::move(secret_key)) {
    InitSession();
}

void SpotClient::InitSession() {
    ctx_.set_default_verify_paths();
    stream_.set_verify_mode(ssl::verify_none);
    stream_.set_verify_callback([](bool preverified, ssl::verify_context& ctx_) {
        return true; 
    });

    if(!SSL_set_tlsext_host_name(stream_.native_handle(), base_url_.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }
    get_lowest_layer(stream_).connect(resolver_.resolve({base_url_, "https"}));
    get_lowest_layer(stream_).expires_after(std::chrono::seconds(3));
    stream_.handshake(ssl::stream_base::client);
}

std::string SpotClient::Response(http::verb type, const std::string& sreq) {
    http::request<http::empty_body> req{type, sreq, 11};
    req.set(http::field::host, base_url_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.insert("X-MBX-APIKEY", api_key_);
    http::write(stream_, req);
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(stream_, buffer, res);
    return beast::buffers_to_string(res.body().data());
}

std::string SpotClient::ping() {
    return Response(http::verb::get, "/api/v3/ping");
}

std::string SpotClient::time() {
    return Response(http::verb::get, "/api/v3/time");
}

std::string SpotClient::exchangeInfo(const std::string& symbol) {
    return Response(http::verb::get, 
    "/api/v3/exchangeInfo?symbol=" + symbol);
}

std::string SpotClient::depth(const std::string& symbol, std::size_t limit) {
    return Response(http::verb::get, 
    "/api/v3/depth?symbol=" + symbol
        + "&limit=" + std::to_string(limit));
}

std::string SpotClient::trades(const std::string& symbol, std::size_t limit) {
    return Response(http::verb::get, 
    "/api/v3/trades?symbol=" + symbol
        + "&limit=" + std::to_string(limit));
}

std::string SpotClient::klines(const std::string& symbol, const std::string& interval) {
    return Response(http::verb::get, 
    "/api/v3/klines?symbol=" + symbol
        + "&interval=" + interval);
}

std::string SpotClient::historicalTrades(const std::string& symbol) {
    return Response(http::verb::get, 
    "/api/v3/historicalTrades?symbol=" + symbol);
}

std::string SpotClient::price(const std::string& symbol) {
    return Response(http::verb::get, 
    "/api/v3/ticker/price?symbol=" + symbol);
}

std::string SpotClient::bookTicker(const std::string& symbol) {
    return Response(http::verb::get, 
    "/api/v3/ticker/bookTicker?symbol=" + symbol);
}

std::string SpotClient::account() {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = "timestamp=" + time + "&recvWindow=50000";
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/account?" + query + signature);
}

std::string SpotClient::openOrders(const std::string& symbol) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = "timestamp=" + time + "&recvWindow=50000" + "&symbol=" + symbol;
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/openOrders?" + query + signature);
}

std::string SpotClient::allOrders(const std::string& symbol) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = "timestamp=" + time + "&recvWindow=50000" + "&symbol=" + symbol;
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/allOrders?" + query + signature);
}

std::string SpotClient::create_new_order_market_test(const std::string& symbol, SideType side, double quantity) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
        "timestamp=" + time + 
        "&recvWindow=50000" + 
        "&symbol=" + symbol + 
        "&side=" + GenSide(side) +
        "&type=MARKET" +
        "&quantity=" + std::to_string(quantity);
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/order/test?" + query + signature);
}

std::string SpotClient::create_new_market_order(const std::string& symbol, SideType side, double quantity) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
        "timestamp=" + time + 
        "&recvWindow=50000" + 
        "&symbol=" + symbol + 
        "&side=" + GenSide(side) +
        "&type=MARKET" +
        "&quantity=" + std::to_string(quantity);
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::post, "/api/v3/order?" + query + signature);
}

std::string SpotClient::GenSide(SideType side) {
    switch (side) {
        case SideType::Buy:
            return "BUY";
        case SideType::Sell:
            return "SELL";
    }
}

OrderHandle SpotClient::create_new_limit_order(const std::string& symbol, SideType side, const std::string& timeInForce, double price, double quantity) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
        "timestamp=" + time + 
        "&recvWindow=50000" + 
        "&symbol=" + symbol + 
        "&side=" + GenSide(side) +
        "&type=LIMIT" +
        "&timeInForce=" + timeInForce + 
        "&price=" + std::to_string(price) + 
        "&quantity=" + std::to_string(quantity);
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    auto ans = Response(http::verb::post, "/api/v3/order?" + query + signature);
    return OrderHandle(std::stoi(getField(ans, "orderId")));
}

std::string SpotClient::info_about_order(const std::string& symbol, OrderHandle handle) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
        "timestamp=" + time + 
        "&orderId=" + std::to_string(handle.GetId()) +
        "&recvWindow=50000" + 
        "&symbol=" + symbol;
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/order?" + query + signature);
}

std::string SpotClient::cancel_order(const std::string& symbol, OrderHandle handle) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
        "timestamp=" + time + 
        "&orderId=" + std::to_string(handle.GetId()) +
        "&recvWindow=50000" + 
        "&symbol=" + symbol;
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::delete_, "/api/v3/order?" + query + signature);
}

}
