#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler app_opened_handler(const std::string& bot_token);

  httplib::Server::Handler new_user_handler(const std::string& bot_token);

  httplib::Server::Handler get_users_handler(const std::string& bot_token);

}
