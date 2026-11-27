#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler new_line_handler(const std::string& bot_token);

  httplib::Server::Handler remove_line_handler(const std::string& bot_token);

  httplib::Server::Handler get_lines_handler(const std::string& bot_token);

}
