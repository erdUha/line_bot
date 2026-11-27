#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/users.hpp"
#include "utils/tg.hpp"

//TODO: proper registration with group password
namespace handlers {

httplib::Server::Handler app_opened_handler(const std::string& bot_token)
{
  return [bot_token](const httplib::Request& req, httplib::Response& res)
  {
    auto json_body = nlohmann::json::parse(req.body);

    try
    {
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
      std::string username = user_json.at("username").get<std::string>();
      std::string first_name = user_json.at("first_name").get<std::string>();
      std::string last_name = user_json.at("last_name").get<std::string>();

      bool is_admin = telegram_id == ADMIN_TG_ID;

      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
            "SELECT first_name FROM users"
            "  WHERE telegram_id = $1 LIMIT 1", telegram_id))
      {
        check_user = q_fn;
      }

      if (check_user.empty())
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "You are not authorized to use the manager! Contact the creator of the bot."},
          {"is_admin", is_admin}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      // No registration!
      //w.exec("INSERT INTO users VALUES ($1, $2, $3, $4)", pqxx::params{telegram_id, username, first_name, last_name});
      //w.commit();

      res.status = 200;
      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Logged in!"},
        {"is_admin", is_admin}
      };
      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "[app_opened_handler] ERROR: " << e.what() << '\n';
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
