#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler app_opened_handler(const Config& config);

  httplib::Server::Handler register_handler(const Config& config);

  httplib::Server::Handler get_users_handler(const Config& config);

}
