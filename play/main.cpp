#include <string>

#include <binana/client.hpp>

#include <iostream>

static const std::string base_url = std::getenv("base_url");
static const std::string api_key = std::getenv("binance_testnet_api_key");
static const std::string secret_key = std::getenv("binance_testnet_secret_key");

int main() {
    binana::SpotClient client(base_url, api_key, secret_key);
    std::cout << client.allOrders("BTCUSDT") << std::endl;
    return 0;
}