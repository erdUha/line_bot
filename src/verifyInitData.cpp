#pragma once

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <ctime>
#include <cstdint>
#include <string>
#include <cstring>
#include <cstdlib>
#include <array>
#include <string_view>
#include <stdexcept>

constexpr std::int64_t MAX_AGE = 3600;
constexpr std::int64_t MAX_FUTURE_SKEW = 60;

std::array<unsigned char, 32> hex_to_bytes(std::string_view hex)
{
  if (hex.size() != 64)
    throw std::invalid_argument("Invalid SHA256 hash length");

  auto hex_digit = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  };

  std::array<unsigned char, 32> bytes{};

  for (size_t i = 0; i < 32; ++i)
  {
    int high = hex_digit(hex[i * 2]);
    int low = hex_digit(hex[i * 2 + 1]);

    if (high < 0 || low < 0)
      throw std::invalid_argument("Invalid hex character");

    bytes[i] = static_cast<unsigned char>((high << 4) | low);
  }

  return bytes;
}

bool verify_init_data(std::string& init_data_str,
                      std::map<std::string, std::string>& fields,
                      const char* bot_token)
{
  std::string data_check_string;

  for (const auto& [key, value] : fields)
  {
    if (key == "hash")
      continue;

    if (!data_check_string.empty())
      data_check_string += '\n';

    data_check_string += key + "=" + value;
  }

  unsigned char secret[EVP_MAX_MD_SIZE];
  unsigned int secret_len = 0;

  HMAC(
    EVP_sha256(),
       "WebAppData",
       10,
       reinterpret_cast<const unsigned char*>(bot_token),
       std::strlen(bot_token),
       secret,
       &secret_len
  );

  unsigned char computed[EVP_MAX_MD_SIZE];
  unsigned int computed_len = 0;

  HMAC(
    EVP_sha256(),
       secret,
       secret_len,
       reinterpret_cast<const unsigned char*>(data_check_string.data()),
       data_check_string.size(),
       computed,
       &computed_len
  );

  auto recieved_hash_bytes = hex_to_bytes(fields["hash"]);

  std::time_t now = std::time(nullptr);
  std::int64_t auth_date = std::stoll(fields["auth_date"]);
  bool invalid_date =
  auth_date < now - MAX_AGE ||
  auth_date > now + MAX_FUTURE_SKEW;

  return
  computed_len == 32 &&
  CRYPTO_memcmp(
    computed,
    recieved_hash_bytes.data(),
    32
  ) == 0 &&
  !invalid_date;
}
