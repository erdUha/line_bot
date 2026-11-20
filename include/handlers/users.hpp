#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler app_opened_handler(const char* bot_token);

  httplib::Server::Handler new_user_handler(const char* bot_token);

}
