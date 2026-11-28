#pragma once

#include <string>
#include <httplib.h>

namespace handlers {

  httplib::Server::Handler new_reservation_handler(const Config& config);

  httplib::Server::Handler remove_reservation_handler(const Config& config);

  httplib::Server::Handler get_reservations_handler(const Config& config);

}
