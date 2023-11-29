#include <client.hpp>
#include <hmac/hmac.hpp>
#include <boost/asio/buffered_read_stream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/version.hpp>
#include <openssl/x509v3.h>
#include <string>

namespace binana {

std::string string_to_hex(const std::string& input) {
    static const char hex_digits[] = "0123456789ABCDEF";
    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input) {
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
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

std::string SpotClient::Response(http::verb type, std::string sreq) {
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

std::string SpotClient::exchangeInfo(std::string symbol) {
    return Response(http::verb::get, 
    "/api/v3/exchangeInfo?symbol=" + symbol);
}

std::string SpotClient::depth(std::string symbol, std::size_t limit) {
    return Response(http::verb::get, 
    "/api/v3/depth?symbol=" + symbol
        + "&limit=" + std::to_string(limit));
}

std::string SpotClient::trades(std::string symbol, std::size_t limit) {
    return Response(http::verb::get, 
    "/api/v3/trades?symbol=" + symbol
        + "&limit=" + std::to_string(limit));
}

std::string SpotClient::klines(std::string symbol, std::string interval) {
    return Response(http::verb::get, 
    "/api/v3/klines?symbol=" + symbol
        + "&interval=" + interval);
}

std::string SpotClient::historicalTrades(std::string symbol) {
    return Response(http::verb::get, 
    "/api/v3/historicalTrades?symbol=" + symbol);
}

std::string SpotClient::price(std::string symbol) {
    return Response(http::verb::get, 
    "/api/v3/ticker/price?symbol=" + symbol);
}

std::string SpotClient::bookTicker(std::string symbol) {
    return Response(http::verb::get, 
    "/api/v3/ticker/bookTicker?symbol=" + symbol);
}

std::string SpotClient::account() {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = "timestamp=" + time + "&recvWindow=50000";
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/account?" + query + signature);
}

std::string SpotClient::openOrders(std::string symbol) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = "timestamp=" + time + "&recvWindow=50000" + "&symbol=" + symbol;
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/openOrders?" + query + signature);
}

std::string SpotClient::allOrders(std::string symbol) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = "timestamp=" + time + "&recvWindow=50000" + "&symbol=" + symbol;
    std::string signature = "&signature=" + hmac::get_hmac(secret_key_, query);
    return Response(http::verb::get, "/api/v3/allOrders?" + query + signature);
}

std::string SpotClient::create_new_order_test(std::string symbol, SideType side, TypeOrder type, double quantity) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
    "timestamp=" + time + 
    "&recvWindow=50000" + 
    "&symbol=" + symbol + 
    "&side=" + GenSide(side) +
    "&type="+ GenType(type) +
    "&quantity=" + std::to_string(quantity);
    std::string hash(' ', EVP_MAX_MD_SIZE);
    unsigned int len = 0;
    unsigned char* result = HMAC(EVP_sha256(), secret_key_.data(), secret_key_.size(),
                        reinterpret_cast<unsigned char*>(query.data()), query.size(), 
                        reinterpret_cast<unsigned char*>(hash.data()), &len);

    std::string signature = "&signature=" + string_to_hex(hash);

    http::request<http::empty_body> req{http::verb::post, "/api/v3/order/test?" + query + signature, 11};
    req.set(http::field::host, base_url_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.insert("X-MBX-APIKEY", api_key_);

    http::write(stream_, req);

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(stream_, buffer, res);        
    return beast::buffers_to_string(res.body().data());
}

std::string SpotClient::create_new_order(std::string symbol, SideType side, TypeOrder type, double quantity) {
    std::string time = std::to_string(std::time(nullptr) * 1000);
    std::string query = 
    "timestamp=" + time + 
    "&recvWindow=50000" + 
    "&symbol=" + symbol + 
    "&side=" + GenSide(side) +
    "&type="+ GenType(type) +
    "&quantity=" + std::to_string(quantity);
    std::string hash(' ', EVP_MAX_MD_SIZE);
    unsigned int len = 0;
    unsigned char* result = HMAC(EVP_sha256(), secret_key_.data(), secret_key_.size(),
                        reinterpret_cast<unsigned char*>(query.data()), query.size(), 
                        reinterpret_cast<unsigned char*>(hash.data()), &len);

    std::string signature = "&signature=" + string_to_hex(hash);

    http::request<http::empty_body> req{http::verb::post, "/api/v3/order?" + query + signature, 11};
    req.set(http::field::host, base_url_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.insert("X-MBX-APIKEY", api_key_);

    http::write(stream_, req);

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(stream_, buffer, res);        
    return beast::buffers_to_string(res.body().data());
} 

std::string SpotClient::info_about_order(std::string symbol, std::size_t order_id) {
    return "";
}

std::string SpotClient::cancel_order(std::string symbol, std::size_t order_id) {
    return "";
}

std::string SpotClient::GenSide(SideType side) {
    switch (side) {
        case SideType::Buy:
            return "BUY";
        case SideType::Sell:
            return "SELL";
    }
}

std::string SpotClient::GenType(TypeOrder type) {
    switch (type) {
        case TypeOrder::MARKET:
            return "MARKET";
        case TypeOrder::LIMIT:
            return "LIMIT";
        case TypeOrder::STOP_LOSS:
            return "STOP_LOSS";
    }
}

}
