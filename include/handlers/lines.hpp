#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler new_line_handler(const Config& config);

  httplib::Server::Handler remove_line_handler(const Config& config);

  httplib::Server::Handler get_lines_handler(const Config& config);

}
