#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler new_line_handler(const char* bot_token);

}
