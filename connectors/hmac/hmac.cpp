#include <cassert>

#include <hmac/hmac.hpp>
#include <openssl/hmac.h>

namespace hmac {

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

    std::string get_hmac(const std::string& key, std::string &msg, const bool is_hex) {
        std::string hash(' ', EVP_MAX_MD_SIZE);
        std::uint32_t len = 0;
        if (!HMAC(EVP_sha256(), key.data(), key.size(),
                        reinterpret_cast<unsigned char*>(msg.data()), msg.size(), 
                        reinterpret_cast<unsigned char*>(hash.data()), &len)) {
                            assert(false);
                        }
        return string_to_hex(hash);
    }

}
