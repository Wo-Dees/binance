#include <binance/client.hpp>

static const std::string base_url = "testnet.binance.vision";



int main() {
    SpotClient client(base_url, testnet_api_key, testnet_secret_key);
    std::cout << client.time() << std::endl;
    return 0;
}