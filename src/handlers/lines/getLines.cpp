#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/lines.hpp"
#include "utils/tg.hpp"

namespace handlers {

nlohmann::json get_one_line(pqxx::work& w, const std::int64_t line_id)
{
  nlohmann::json select_line;

  for (auto [q_li, q_cn, q_ln, q_dd, q_ti, q_fn] :
        w.query<int64_t,
                std::string,
                std::string,
                std::string,
                std::int64_t,
                std::string>(
          "SELECT l.line_id, l.course_name,"
          "  l.line_name, l.defence_date, u.telegram_id, u.first_name"
          "  FROM lines l"
          "  JOIN users u"
          "  ON l.creator_id = u.telegram_id"
          "  WHERE l.line_id = $1 LIMIT 1",
          line_id
        )
      )
  {
    select_line = {
      {"line_id", q_li},
      {"course_name", q_cn},
      {"line_name", q_ln},
      {"defence_date", q_dd},
      {"creator_id", q_ti},
      {"creator_name", q_fn}
    };
  }

  return select_line;
}

nlohmann::json get_multiple_lines(pqxx::work& w)
{
  nlohmann::json select_lines = nlohmann::json::array();

  for (auto [q_li, q_cn, q_ln, q_dd, q_igw, q_fn] :
        w.query<int64_t,
                std::string,
                std::string,
                std::string,
                bool,
                std::string>(
          "SELECT l.line_id, l.course_name, l.line_name, l.defence_date, l.is_group_work, u.first_name"
          "  FROM lines l"
          "  JOIN users u"
          "  ON l.creator_id = u.telegram_id"
          "  WHERE l.defence_date >= CURRENT_DATE"
          "  ORDER BY l.defence_date"
        )
      )
  {
    select_lines.push_back({
      {"line_id", q_li},
      {"course_name", q_cn},
      {"line_name", q_ln},
      {"defence_date", q_dd},
      {"is_group_work", q_igw},
      {"creator_first_name", q_fn}
    });
  }

  return select_lines;
}

httplib::Server::Handler get_lines_handler(const char* bot_token)
{
  return [bot_token](const httplib::Request& req, httplib::Response& res)
  {
    try
    {
      auto json_body = nlohmann::json::parse(req.body);

      std::string init_data_str = json_body.at("initData");

      httplib::Params params;
      httplib::detail::parse_query_text(init_data_str, params);
      std::map<std::string, std::string> fields;
      for (const auto& [key, value] : params) {
        fields[key] = value;
      }

      bool valid = utils::verify_init_data(init_data_str, fields, bot_token);

      if (!valid)
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "Your hash wasn't verified or request expired!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      const auto user_json = nlohmann::json::parse(fields["user"]);

      const std::int64_t user_telegram_id = user_json.at("id").get<std::int64_t>();

      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT first_name FROM users WHERE telegram_id = $1 LIMIT 1",
              user_telegram_id
            )
          )
      {
        check_user = q_fn;
      }

      if (check_user.empty())
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "You are not authorized to view lines!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      nlohmann::json responseObj;

      if (json_body.contains("line_id"))
      {
        nlohmann::json select_one_line = get_one_line(
          w,
          json_body.at("line_id").get<std::int64_t>()
        );

        responseObj = {
          {"status", "success"},
          {"message", "OK"},
          {"line", select_one_line}
        };
      }
      else
      {
        nlohmann::json select_multiple_lines = get_multiple_lines(w);

        responseObj = {
          {"status", "success"},
          {"message", "OK"},
          {"lines", select_multiple_lines}
        };
      }

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "[get_lines_handler] ERROR: " << e.what() << '\n';
      res.status = 500;
      nlohmann::json responseObj = {
        {"status", "failure"},
        {"message", "Internal server error"}
      };
      res.set_content(responseObj.dump(), "application/json");
      return;
    }
  };
}

}
