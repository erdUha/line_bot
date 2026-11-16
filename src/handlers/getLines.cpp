#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.h"
#include "handlers/lines.h"

namespace handlers {

nlohmann::json getOneLine(pqxx::work& w, std::string line_id)
{
  nlohmann::json select_line;

  for (auto [q_li, q_cn, q_ln, q_dd, q_ci] :
        w.query<long long,
                std::string,
                std::string,
                std::string,
                long long>(
          "SELECT line_id, course_name, line_name, defence_date, creator_id FROM lines WHERE line_id = $1 LIMIT 1",
          std::stoll(line_id)
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
    select_line = {
      {"line_id", q_li},
      {"course_name", q_cn},
      {"line_name", q_ln},
      {"defence_date", q_dd},
      {"creator_username", username}
    };
  }

  return select_line;
}

nlohmann::json getMultipleLines(pqxx::work& w)
{
  nlohmann::json select_lines = nlohmann::json::array();

  for (auto [q_li, q_cn, q_ln, q_dd, q_ci] :
        w.query<long long,
                std::string,
                std::string,
                std::string,
                long long>(
          "SELECT line_id, course_name, line_name, defence_date, creator_id FROM lines WHERE defence_date >= CURRENT_DATE"
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
      {"line_id", q_li},
      {"course_name", q_cn},
      {"line_name", q_ln},
      {"defence_date", q_dd},
      {"creator_username", username}
    });
  }

  return select_lines;
}

httplib::Server::Handler get_lines_handler()
{
  return [](const httplib::Request& req, httplib::Response& res)
  {
    // Only for one line
    std::string param_line_id = "";

    if (req.has_param("line_id"))
      param_line_id = req.get_param_value("line_id");

    try
    {
      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};
      nlohmann::json responseObj;

      if (!param_line_id.empty())
      {
        nlohmann::json select_one_line = getOneLine(w, param_line_id);

        responseObj = {
          {"status", "success"},
          {"message", "Test"},
          {"line", select_one_line}
        };
      }
      else
      {
        nlohmann::json select_multiple_lines = getMultipleLines(w);

        responseObj = {
          {"status", "success"},
          {"message", "Test"},
          {"lines", select_multiple_lines}
        };
      }

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
