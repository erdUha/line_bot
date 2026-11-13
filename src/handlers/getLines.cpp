#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.h"
#include "handlers/lines.h"

namespace handlers {

httplib::Server::Handler get_lines_handler()
{
  return [](const httplib::Request& req, httplib::Response& res)
  {
    try
    {
      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      nlohmann::json select_lines = nlohmann::json::array();

      for (auto [q_cn, q_ln, q_dd, q_ci] :
            w.query<std::string,
                    std::string,
                    std::string,
                    long long>(
              "SELECT course_name, line_name, defence_date, creator_id FROM lines"
            )
           )
      {
        std::string username = "";
        for (auto [q_un] :
              w.query<std::string>(
                "SELECT username FROM users WHERE telegram_id = $1 LIMIT 1",
                q_ci
              )
            )
        {
          username = q_un;
        }
        select_lines.push_back({
          {"course_name", q_cn},
          {"line_name", q_ln},
          {"defence_date", q_dd},
          {"creator_username", username}
        });
      }

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Test"},
        {"lines", select_lines}
      };

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "ERROR: " << e.what() << '\n';
      res.status = 500;
      nlohmann::json responseObj = {
        {"status", "failure"},
        {"message", "Server error!"}
      };
      res.set_content(responseObj.dump(), "application/json");
      return;
    }
  };
}

}
