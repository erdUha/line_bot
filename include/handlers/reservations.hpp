#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler new_reservation_handler(const std::string& bot_token);

  httplib::Server::Handler remove_reservation_handler(const std::string& bot_token);

  httplib::Server::Handler get_reservations_handler(const std::string& bot_token);

}
