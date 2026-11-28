#include "config.hpp"

#include <cstdlib>
#include <stdexcept>

std::string Config::require_env(const char* name)
{
  const char* value = std::getenv(name);

  if (!value || !*value)
    throw std::runtime_error(
      std::string("Missing environment variable: ") + name
    );

  return value;
}

Config::Config()
  : port(std::stoi(require_env("PORT"))),
    bot_token(require_env("LINE_BOT_TOKEN")),
    database_url(require_env("DATABASE_URL")),
    admin_tg_id(std::stoll(require_env("ADMIN_TG_ID"))),
    admin_username(require_env("ADMIN_USERNAME")),
    group_password_hash(require_env("GROUP_PASSWORD_HASH"))
{
  if (port < 1 || port > 65535)
    throw std::runtime_error("Invalid PORT");
}

const Config& Config::get()
{
  static const Config instance;
  return instance;
}
