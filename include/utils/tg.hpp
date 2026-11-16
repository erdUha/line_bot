#pragma once

#include <array>
#include <string>
#include <string_view>
#include <map>

namespace utils {

  std::array<unsigned char, 32> hex_to_bytes(const std::string_view& hex);

  bool verify_init_data(const std::string& init_data_str,
                        const std::map<std::string, std::string>& fields,
                        const char* bot_token);

}
