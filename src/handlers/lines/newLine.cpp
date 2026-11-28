#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <sstream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/lines.hpp"
#include "utils.hpp"

namespace handlers {

bool is_valid_date(const std::string& date_str) {
  if (date_str.size() != 10) return false;

  std::istringstream ss{date_str};
  std::chrono::year_month_day ymd{};
  ss >> std::chrono::parse("%F", ymd);

  if (ss.fail() || ss.peek() != EOF || !ymd.ok()) {
    return false;
  }

  auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
  return std::chrono::sys_days{ymd} >= today;
}

httplib::Server::Handler new_line_handler(const Config& config)
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

      std::int64_t creator_id = user_json.at("id").get<std::int64_t>();

      pqxx::connection cx(config.database_url);

      pqxx::work w{cx};

      std::string check_user = "";

      for (auto [q_fn] : w.query<std::string>(
              "SELECT first_name FROM users"
              "  WHERE telegram_id = $1 LIMIT 1",
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
          {"message", "You are not authorized to create lines!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      std::string course_name = json_body.at("course_name").get<std::string>();
      std::string line_name = json_body.at("line_name").get<std::string>();
      bool is_group_work = json_body.at("is_group_work").get<bool>();
      std::string defence_date = json_body.at("defence_date").get<std::string>();

      bool is_input_fail = false;
      std::string fail_message;

      if (course_name.size() < 2)
      {
        is_input_fail = true;
        fail_message = "Course name should contain at least 2 characters!";
      }
      if (course_name.size() > 255)
      {
        is_input_fail = true;
        fail_message = "Course name length shouldn't exceed 255!";
      }
      if (line_name.size() < 2)
      {
        is_input_fail = true;
        fail_message = "Line name should contain at least 2 characters!";
      }
      if (line_name.size() > 255)
      {
        is_input_fail = true;
        fail_message = "Line name length shouldn't exceed 255!";
      }
      if (!is_valid_date(defence_date))
      {
        is_input_fail = true;
        fail_message = "Assignment date is invalid or before today!";
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


      w.exec("INSERT INTO lines"
             "  (course_name, line_name, is_group_work, defence_date, creator_id)"
             "  VALUES ($1, $2, $3, $4, $5)",
             pqxx::params{
               course_name,
               line_name,
               is_group_work,
               defence_date,
               creator_id
             });

      w.commit();

      std::cout << "Line " << line_name << " has been successfully created!\n";

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Successfully created the line!"}
      };

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "[new_line_handler] ERROR: " << e.what() << '\n';
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
