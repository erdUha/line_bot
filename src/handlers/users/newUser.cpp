#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/users.hpp"
#include "utils/tg.hpp"

namespace handlers {

httplib::Server::Handler new_user_handler(const std::string& bot_token)
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

      std::int64_t telegram_id = user_json.at("id").get<std::int64_t>();

      bool is_admin = telegram_id == ADMIN_TG_ID;

      if (!is_admin)
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "You should be the admin to add new users!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      const auto new_user = json_body.at("new_user");
      std::int64_t new_tg_id = new_user.at("telegram_id").get<std::int64_t>();
      std::string new_username = new_user.at("username").get<std::string>();
      std::string new_first_name = new_user.at("first_name").get<std::string>();
      std::string new_last_name = new_user.at("last_name").get<std::string>();

      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT first_name FROM users"
              "  WHERE telegram_id = $1 LIMIT 1",
              new_tg_id
            )
          )
      {
        check_user = q_fn;
      }

      if (!check_user.empty())
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "User is already in the DB!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      w.exec("INSERT INTO users"
             "  VALUES ($1, $2, $3, $4)",
             pqxx::params{
                 new_tg_id,
                 new_username,
                 new_first_name,
                 new_last_name
               }
             );
      w.commit();

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "New user was registered successfully!"}
      };

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "[new_user_handler] ERROR: " << e.what() << '\n';
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
