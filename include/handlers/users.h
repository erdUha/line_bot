#pragma once

#include <string>
#include <httplib.h>

const std::string conn_str = "dbname=line user=postgres password=postgres host=127.0.0.1 port=5432";

namespace handlers {

  httplib::Server::Handler app_opened_handler(const char* bot_token);

}
