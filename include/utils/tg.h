#pragma once

#include <array>
#include <string>
#include <string_view>
#include <map>

namespace utils {

  std::array<unsigned char, 32> hex_to_bytes(std::string_view hex);

  bool verify_init_data(std::string& init_data_str,
                        std::map<std::string, std::string>& fields,
                        const char* bot_token);

}
