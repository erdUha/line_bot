#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/users.hpp"
#include "utils.hpp"

namespace handlers {

httplib::Server::Handler register_handler(const Config& config)
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

      std::int64_t telegram_id = user_json.at("id").get<std::int64_t>();
      std::string username = user_json.at("username").get<std::string>();

      std::string group_password = json_body.at("group_password").get<std::string>();
      std::string first_name = json_body.at("first_name").get<std::string>();
      std::string last_name = json_body.at("last_name").get<std::string>();

      bool is_input_fail = false;
      std::string fail_message;

      if (!utils::verify_bcrypt_hash(group_password, config.group_password_hash))
      {
        is_input_fail = true;
        fail_message = "Wrong group password!";
      }
      if (first_name.size() < 2)
      {
        is_input_fail = true;
        fail_message = "First name is too short! It's should be 2 characters or more!";
      }
      if (first_name.size() > 20)
      {
        is_input_fail = true;
        fail_message = "First name is too long! It's shouldn't exceed 20 characters!";
      }
      if (!last_name.empty() && last_name.size() < 2)
      {
        is_input_fail = true;
        fail_message = "Last name is too short! It's should be 2 characters or more!";
      }
      if (last_name.size() > 20)
      {
        is_input_fail = true;
        fail_message = "Last name is too long! It's shouldn't exceed 20 characters!";
      }

      if (is_input_fail)
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", fail_message}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      pqxx::connection cx(config.database_url);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT first_name FROM users"
              "  WHERE telegram_id = $1 LIMIT 1",
              telegram_id
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
          {"message", "User is already in the Database!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      w.exec("INSERT INTO users"
             "  VALUES ($1, $2, $3, $4)",
             pqxx::params{
                 telegram_id,
                 username,
                 first_name,
                 last_name
               }
             );
      w.commit();

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Registered successfully!"}
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
