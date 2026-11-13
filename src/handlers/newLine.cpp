#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.h"
#include "handlers/lines.h"
#include "utils/tg.h"

namespace handlers {

httplib::Server::Handler new_line_handler(const char* bot_token)
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
        std::cerr << "User's hash was not verified or expired!";
        res.status = 401;
        res.set_content("Your hash wasn't verified or request expired!", "text/plain");
        return;
      }

      const auto user_json = nlohmann::json::parse(fields["user"]);

      std::int64_t creator_id = user_json.at("id").get<std::int64_t>();

      std::string course_name = json_body.at("course_name").get<std::string>();
      std::string line_name = json_body.at("line_name").get<std::string>();
      std::string defence_date = json_body.at("defence_date").get<std::string>();

      if (course_name.size() > 255)
      {
        res.status = 401;
        nlohmann::json responseObj = {
          {"status", "failure"},
          {"message", "Course name length shouldn't exceed 255!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      std::cout << "Line creation started!\n";

      pqxx::connection cx(DATABASE_URL);

      pqxx::work w{cx};

      w.exec("INSERT INTO lines"
             "  (course_name, line_name, defence_date, creator_id)"
             "  VALUES ($1, $2, $3, $4)",
             pqxx::params{
               course_name,
               line_name,
               defence_date,
               creator_id
             });
      w.commit();

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Successfully create a line!"}
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
