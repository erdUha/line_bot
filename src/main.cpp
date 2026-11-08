#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

const std::string conn_str = "dbname=line user=postgres password=postgres host=127.0.0.1 port=5432";

int main()
{
  httplib::Server svr;

  try
  {
    pqxx::connection cx(conn_str);

    pqxx::work creating_tables(cx);
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
      "   line_name VARCHAR(255) NOT NULL"
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

  svr.Post("/api/app_opened", [](const httplib::Request& req, httplib::Response& res) {
    auto json_body = nlohmann::json::parse(req.body);

    try
    {
      int telegram_id = json_body["telegram_id"];
      std::string username = json_body["username"];
      std::string first_name = json_body["first_name"];
      std::string last_name = json_body["last_name"];
      std::cout << "telegram_id: " << telegram_id << '\n';

      pqxx::connection cx(conn_str);
      std::cout << "Connected to " << cx.dbname() << '\n';

      pqxx::work checking_user{cx};
      for (auto [telegram_id_pq, username_pq] : checking_user.query<long long, std::string>(
          "SELECT telegram_id, username FROM users ORDER BY username"))
      {
          std::cout << telegram_id_pq << ": " << username_pq << ".\n";
      }

    }
    catch (std::exception const &e)
    {
      std::cerr << "ERROR: " << e.what() << '\n';
      res.status = 500;
      res.set_content("Database connection failed", "text/plain");
      return;
    }

    nlohmann::json responseObj = {
      {"status", "success"},
      {"message", "Success"}
    };

    res.set_content(responseObj.dump(), "application/json");
  });

  std::cout << "Listening on 0.0.0.0:8080...\n";
  svr.listen("0.0.0.0", 8080);

  return 0;
}
