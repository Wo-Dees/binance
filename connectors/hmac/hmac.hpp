#pragma once

#include <string>

namespace hmac {

std::string get_hmac(std::string key, const std::string &msg, const bool is_hex = true);

}