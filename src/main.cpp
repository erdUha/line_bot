#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.h"
#include "handlers/all_handlers.h"

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
    pqxx::connection cx(config::conn_str);

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
      "   course_name VARCHAR(255) NOT NULL,"
      "   line_name VARCHAR(255),"
      "   defence_date DATE NOT NULL,"
      "   creator_id BIGINT NOT NULL,"
      "   CONSTRAINT fk_user"
      "     FOREIGN KEY (creator_id)"
      "     REFERENCES users(telegram_id)"
      "     ON DELETE SET NULL"
      ");"
    );

    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS seats ("
      "   seat_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
      "   line_id BIGINT NOT NULL,"
      "   booked_by_user BIGINT NOT NULL,"
      "   CONSTRAINT fk_line"
      "     FOREIGN KEY (line_id)"
      "     REFERENCES lines(line_id)"
      "     ON DELETE CASCADE,"
      "   CONSTRAINT fk_user"
      "     FOREIGN KEY (booked_by_user)"
      "     REFERENCES users(telegram_id)"
      "     ON DELETE SET NULL"
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

  svr.Post("/api/app_opened",
           handlers::app_opened_handler(bot_token));

  svr.Post("/api/new_line",
           handlers::new_line_handler(bot_token));

  std::cout << "Listening on 0.0.0.0:8080...\n";
  svr.listen("0.0.0.0", 8080);

  return 0;
}
