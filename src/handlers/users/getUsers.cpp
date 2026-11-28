#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/lines.hpp"
#include "utils.hpp"

namespace handlers {

nlohmann::json get_all_users(pqxx::work& w)
{
  nlohmann::json select_users = nlohmann::json::array();

  for (auto [q_un, q_fn, q_ln] :
        w.query< std::string,
                std::string,
                std::string>(
          "SELECT"
          "  COALESCE(username, ''),"
          "  first_name,"
          "  COALESCE(last_name, '')"
          "  FROM users"
        )
      )
  {
    select_users.push_back({
      {"username", q_un},
      {"first_name", q_fn},
      {"last_name", q_ln}
    });
  }

  return select_users;
}

httplib::Server::Handler get_users_handler(const Config& config)
{
  return [config](const httplib::Request& req, httplib::Response& res)
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

      bool valid = utils::verify_init_data(init_data_str, fields, config.bot_token);

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

      pqxx::connection cx(config.database_url);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT first_name FROM users"
              "  WHERE telegram_id = $1 LIMIT 1",
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
          {"message", "You are not authorized to view users!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      nlohmann::json responseObj;

      nlohmann::json select_all_users = get_all_users(w);

      responseObj = {
        {"status", "success"},
        {"message", "OK"},
        {"users", select_all_users}
      };

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "[get_users_handler] ERROR: " << e.what() << '\n';
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
