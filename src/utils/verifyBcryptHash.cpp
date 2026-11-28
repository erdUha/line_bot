#include <crypt.h>
#include <cstring>
#include <string>

#include "utils.hpp"

namespace utils {

bool verify_bcrypt_hash(const std::string& password, const std::string& stored_hash)
{
  if (stored_hash.empty() || password.empty()) {
    return false;
  }

  struct crypt_data data {};
  data.initialized = 0;

  const char* computed = crypt_r(password.c_str(), stored_hash.c_str(), &data);
  if (!computed || computed[0] == '*') {
    return false;
  }

  const size_t stored_len = stored_hash.size();
  if (std::strlen(computed) != stored_len) {
    return false;
  }

  unsigned char diff = 0;
  for (size_t i = 0; i < stored_len; ++i) {
    diff |= static_cast<unsigned char>(computed[i]) ^ static_cast<unsigned char>(stored_hash[i]);
  }
  return diff == 0;
}

}
