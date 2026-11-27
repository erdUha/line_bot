#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/reservations.hpp"
#include "utils/tg.hpp"

namespace handlers {

httplib::Server::Handler new_reservation_handler(const char* bot_token)
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

      std::uint64_t creator_id = user_json.at("id").get<std::int64_t>();

      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT first_name FROM users "
              "WHERE telegram_id = $1 LIMIT 1",
              creator_id
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
          {"message", "You are not authorized to reserve spots!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      const std::int64_t line_id = json_body.at("line_id").get<std::int64_t>();

      bool is_group_work = false;

      // Locking
      for (auto [group_work] : w.query<bool>(
        "SELECT is_group_work FROM lines "
        "WHERE line_id = $1 FOR UPDATE",
        line_id
      ))
      {
        is_group_work = group_work;
      }

      for (auto&& unused : w.query<int>(
              "SELECT position FROM reservations "
              "  WHERE reserved_by_id = $1"
              "  AND   line_id = $2",
              pqxx::params{
                creator_id,
                line_id
              }
            )
          )
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "You already have a reservation!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      const int position = json_body.at("position").get<int>();

      std::int64_t user_count = w.query_value<std::int64_t>(
        "SELECT COUNT(*) FROM users"
      );

      if (position < 1)
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "Spot number is out of range! It cannot be 0 or negative."}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }
      else if (position > user_count)
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "Spot number is out of range! It cannot exceed number of users."}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      if (!is_group_work)
      {
        for (auto&& unused : w.query<int>(
          "SELECT position FROM reservations"
          "  WHERE position = $1"
          "  AND line_id = $2",
          pqxx::params{
            position,
            line_id
          }
        ))
        {
          res.status = 401;
          nlohmann::json responseObj = {
            {"status", "failure"},
            {"message", "It's not a group work! Please, pick a free spot."}
          };
          res.set_content(responseObj.dump(), "application/json");
          return;
        }
      }

      w.exec("INSERT INTO reservations"
             "  (line_id, position, reserved_by_id)"
             "  VALUES ($1, $2, $3)",
             pqxx::params{
               line_id,
               position,
               creator_id
             });

      w.commit();

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Successfully reserved the spot!"}
      };

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "[new_reservation_handler] ERROR: " << e.what() << '\n';
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
