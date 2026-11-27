#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/lines.hpp"
#include "utils/tg.hpp"

namespace handlers {

nlohmann::json get_one_reservation(pqxx::work& w, const std::int64_t& line_id, const std::int64_t& user_tg_id)
{
  nlohmann::json select_reservation = {};

  for (auto [q_p, q_fn] : w.query<int, std::string>(
          "SELECT r.position, u.first_name"
          "  FROM reservations r"
          "  JOIN users u"
          "  ON r.reserved_by_id = u.telegram_id"
          "  WHERE r.line_id = $1"
          "  AND   u.telegram_id = $2"
          "  ORDER BY r.position LIMIT 1",
          pqxx::params{
            line_id,
            user_tg_id
          }
        )
      )
  {
    select_reservation = {
      {"position", q_p},
      {"reserved_by", q_fn}
    };
  }

  return select_reservation;
}

nlohmann::json get_all_reservations(pqxx::work& w, std::int64_t line_id)
{
  nlohmann::json select_reservations = nlohmann::json::array();

  for (auto [q_p, q_u] : w.query<int, std::string>(
          "SELECT r.position,"
          "  json_agg("
          "    json_build_object("
          "      'first_name', u.first_name,"
          "      'last_name', u.last_name"
          "    )"
          "  )"
          "  FROM reservations r"
          "  JOIN users u"
          "  ON r.reserved_by_id = u.telegram_id"
          "  WHERE r.line_id = $1"
          "  GROUP BY r.position"
          "  ORDER BY r.position",
          line_id
        )
      )
  {
    select_reservations.push_back({
      {"position", q_p},
      {"reserved_by", nlohmann::json::parse(q_u)}
    });
  }

  return select_reservations;
}

httplib::Server::Handler get_reservations_handler(const std::string& bot_token)
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

      std::int64_t user_telegram_id = user_json.at("id").get<std::int64_t>();

#ifdef DEBUG
      user_telegram_id++;
#endif

      pqxx::connection cx(DATABASE_URL);

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

      const std::int64_t line_id = json_body.at("line_id").get<std::int64_t>();

      nlohmann::json responseObj;

      if (json_body.contains("is_my_reservation") &&
          json_body.at("is_my_reservation").get<bool>() == true)
      {
        nlohmann::json select_one_reservation = get_one_reservation(w, line_id, user_telegram_id);

        responseObj = {
          {"status", "success"},
          {"message", "OK"},
          {"reservation", select_one_reservation}
        };
      }
      else
      {
        nlohmann::json select_all_reservations = get_all_reservations(w, line_id);
        nlohmann::json select_one_reservation = get_one_reservation(w, line_id, user_telegram_id);

        std::int64_t user_count = w.query_value<std::int64_t>(
          "SELECT COUNT(*) FROM users"
        );

        responseObj = {
          {"status", "success"},
          {"message", "OK"},
          {"reservations", select_all_reservations},
          {"my_reservation", select_one_reservation},
          {"user_count", user_count}
        };
      }

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
