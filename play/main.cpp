#include "binana/order.hpp"
#include <string>

#include <binana/client.hpp>

#include <iostream>

static const std::string base_url = std::getenv("base_url");
static const std::string api_key = std::getenv("binance_testnet_api_key");
static const std::string secret_key = std::getenv("binance_testnet_secret_key");

int main() {
    binana::SpotClient client(base_url, api_key, secret_key);
    std::cout << "My open orders " << std::endl;
    std::cout << client.openOrders("BTCUSDT") << std::endl;

    auto handle = client.create_new_limit_order("BTCUSDT", binana::SideType::Sell, "GTC", 50000, 0.3);

    std::cout << "My open orders " << std::endl;
    std::cout << client.openOrders("BTCUSDT") << std::endl;

    std::cout << "Get info about new order " << std::endl;
    std::cout << client.info_about_order("BTCUSDT", handle) << std::endl;
    std::cout << "Cancel new order " << std::endl;
    std::cout << client.cancel_order("BTCUSDT", handle) << std::endl;

    std::cout << "My open orders " << std::endl;
    std::cout << client.openOrders("BTCUSDT") << std::endl;

    return 0;
}