#pragma once

#include <string>

namespace hmac {

std::string get_hmac(const std::string& key, std::string &msg, const bool is_hex = true);

}