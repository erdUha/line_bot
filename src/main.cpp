#include <iostream>
#include <string>
#include <map>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "verifyInitData.cpp"

const std::string conn_str = "dbname=line user=postgres password=postgres host=127.0.0.1 port=5432";

int main()
{
  const char* bot_token = std::getenv("LINE_BOT_TOKEN");

  if (bot_token == nullptr || bot_token[0] == '\0')
  {
    std::cerr << "LINE_BOT_TOKEN is not set!\n";
    return 1;
  }

  httplib::Server svr;

  try
  {
    pqxx::connection cx(conn_str);

    pqxx::work creating_tables{cx};
    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS users ("
      "   telegram_id BIGINT PRIMARY KEY,"
      "   username VARCHAR(255) UNIQUE,"
      "   first_name VARCHAR(255) NOT NULL,"
      "   last_name VARCHAR(255)"
      ");"
    );

    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS lines ("
      "   line_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
      "   line_name VARCHAR(255) NOT NULL,"
      "   line_date DATE NOT NULL"
      ");"
    );

    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS seats ("
      "   seat_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
      "   line_id BIGINT NOT NULL,"
      "   booked_by_user BIGINT,"
      "   CONSTRAINT fk_line FOREIGN KEY (line_id) REFERENCES lines(line_id) ON DELETE CASCADE,"
      "   CONSTRAINT fk_user FOREIGN KEY (booked_by_user) REFERENCES users(telegram_id) ON DELETE SET NULL"
      ");"
    );
    creating_tables.commit();
    std::cout << "Tables were created successfully!\n";
  }
  catch (std::exception const &e)
  {
    std::cerr << "Startup DB Error: " << e.what() << '\n';
    return 1;
  }

  auto ret = svr.set_mount_point("/", "./public");
  if (!ret)
  {
    std::cerr << "The 'public' directory doesn't exist!" << std::endl;
    return 1;
  }

  svr.Post("/webhook", [](const httplib::Request &req, httplib::Response &res) {
    std::cout << "Recieved update:\n" << req.body << std::endl;

    res.set_content("OK", "text/plain");
  });

  svr.Post("/api/app_opened", [bot_token](const httplib::Request& req, httplib::Response& res) {
    auto json_body = nlohmann::json::parse(req.body);

    try
    {
      std::string init_data_str = json_body["initData"];

      httplib::Params params;
      httplib::detail::parse_query_text(init_data_str, params);
      std::map<std::string, std::string> fields;
      for (const auto& [key, value] : params) {
        fields[key] = value;
      }

      bool valid = verify_init_data(init_data_str, fields, bot_token);

      if (!valid)
      {
        std::cerr << "User's hash was not verified or expired!";
        res.status = 401;
        res.set_content("Your hash wasn't verified or request expired!", "text/plain");
        return;
      }

      const auto user_json = nlohmann::json::parse(fields["user"]);

      std::int64_t telegram_id = user_json.at("id").get<std::int64_t>();
      std::string username = user_json.at("username").get<std::string>();
      std::string first_name = user_json.at("first_name").get<std::string>();
      std::string last_name = user_json.at("last_name").get<std::string>();

      std::cout << username << " has opened the app!\n";

      pqxx::connection cx(conn_str);

      pqxx::work w{cx};

      std::string check_username = "";

      for (auto [q_un] : w.query<std::string>(
            "SELECT username FROM users WHERE telegram_id = $1 LIMIT 1", telegram_id))
      {
        check_username = q_un;
      }

      if (!check_username.empty())
      {
        res.status = 200;
        nlohmann::json responseObj = {
          {"status", "success"},
          {"message", "Logged in!"}
        };
        res.set_content(responseObj.dump(), "application/json");
        return;
      }

      w.exec("INSERT INTO users VALUES ($1, $2, $3, $4)", pqxx::params{telegram_id, username, first_name, last_name});
      w.commit();

      nlohmann::json responseObj = {
        {"status", "success"},
        {"message", "Registered!"}
      };

      res.set_content(responseObj.dump(), "application/json");
    }
    catch (std::exception const &e)
    {
      std::cerr << "ERROR: " << e.what() << '\n';
      res.status = 500;
      res.set_content("Database connection failed", "text/plain");
      return;
    }
  });

  std::cout << "Listening on 0.0.0.0:8080...\n";
  svr.listen("0.0.0.0", 8080);

  return 0;
}
