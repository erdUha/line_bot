#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/lines.hpp"
#include "utils/tg.hpp"

namespace handlers {

httplib::Server::Handler remove_line_handler(const std::string& bot_token)
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

      std::int64_t creator_id = user_json.at("id").get<std::int64_t>();

      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      const std::int64_t line_id = json_body.at("line_id").get<std::int64_t>();

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT u.first_name"
              "  FROM users u"
              "  JOIN lines l"
              "  ON u.telegram_id = l.creator_id"
              "  WHERE u.telegram_id = $1"
              "  AND   l.line_id = $2 LIMIT 1",
              pqxx::params{
                creator_id,
                line_id
              }
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
          {"message", "You are not authorized to remove this line!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      w.exec("DELETE FROM lines"
             "  WHERE creator_id = $1"
             "  AND   line_id = $2",
             pqxx::params{
                 creator_id,
                 line_id
               }
             );

      w.commit();

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Successfully removed line!"}
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
