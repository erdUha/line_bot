#pragma once

#include <string>
#include <map>

namespace utils {

  bool verify_init_data(const std::string& init_data_str,
                        const std::map<std::string, std::string>& fields,
                        const std::string& bot_token);

  bool verify_bcrypt_hash(const std::string& password, const std::string& stored_hash);

}
