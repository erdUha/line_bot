#pragma once

#include <string>
#include <cstdint>

class Config {
public:
  static const Config& get();

  int port;
  std::string bot_token;
  std::string database_url;
  std::int64_t admin_tg_id;
  std::string admin_username;
  std::string group_password_hash;

private:
  Config();

  static std::string require_env(const char* name);
};
