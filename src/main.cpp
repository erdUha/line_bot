#include <iostream>
#include <string>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "config.hpp"
#include "handlers/all_handlers.hpp"

int main()
{
  httplib::Server svr;

  svr.set_payload_max_length(1024 * 1024); // 1 MiB
  svr.set_read_timeout(5, 0);
  svr.set_write_timeout(5, 0);

  try
  {
    pqxx::connection cx(DATABASE_URL);

    pqxx::work creating_tables{cx};
    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS users ("
      "  telegram_id BIGINT PRIMARY KEY,"
      "  username VARCHAR(255) UNIQUE,"
      "  first_name VARCHAR(255) NOT NULL,"
      "  last_name VARCHAR(255)"
      ");"
    );

    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS lines ("
      "  line_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
      "  course_name VARCHAR(255) NOT NULL,"
      "  line_name VARCHAR(255),"
      "  is_group_work BOOLEAN NOT NULL,"
      "  defence_date DATE NOT NULL,"
      "  creator_id BIGINT,"
      "  CONSTRAINT fk_user"
      "    FOREIGN KEY (creator_id)"
      "    REFERENCES users(telegram_id)"
      "    ON DELETE SET NULL"
      ");"
    );

    creating_tables.exec(
      "CREATE TABLE IF NOT EXISTS reservations ("
      "  spot_id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,"
      "  line_id BIGINT NOT NULL,"
      "  position INT NOT NULL,"
      "  reserved_by_id BIGINT NOT NULL,"
      "  CONSTRAINT fk_line"
      "    FOREIGN KEY (line_id)"
      "    REFERENCES lines(line_id)"
      "    ON DELETE CASCADE,"
      "  CONSTRAINT fk_user"
      "    FOREIGN KEY (reserved_by_id)"
      "    REFERENCES users(telegram_id)"
      "    ON DELETE CASCADE,"
      "  CONSTRAINT unique_user_reservation"
      "    UNIQUE (line_id, reserved_by_id),"
      "  CONSTRAINT positive_position"
      "    CHECK (position > 0)"
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

  auto ret_styles = svr.set_mount_point("/styles", "./public/styles");
  if (!ret_styles)
  {
    std::cerr << "The './public/styles' directory doesn't exist!" << std::endl;
    return 1;
  }

  auto ret_scripts = svr.set_mount_point("/scripts", "./public/js");
  if (!ret_scripts)
  {
    std::cerr << "The './public/js' directory doesn't exist!" << std::endl;
    return 1;
  }

  svr.Get("/", [](const auto& req, auto& res) {
    res.set_file_content("./public/html/index.html");
  });

  svr.Get("/line", [](const auto& req, auto& res) {
    res.set_file_content("./public/html/line.html");
  });

  svr.Post("/webhook", [](const httplib::Request &req, httplib::Response &res) {
    std::cout << "Recieved update:\n" << req.body << std::endl;

    res.set_content("OK", "text/plain");
  });

  svr.Post("/api/app_opened",
           handlers::app_opened_handler(BOT_TOKEN));

  svr.Post("/api/admin/new_user",
           handlers::new_user_handler(BOT_TOKEN));

  svr.Post("/api/get_users",
           handlers::get_users_handler(BOT_TOKEN));

  svr.Post("/api/new_line",
           handlers::new_line_handler(BOT_TOKEN));

  svr.Post("/api/get_lines",
           handlers::get_lines_handler(BOT_TOKEN));

  svr.Post("/api/new_reservation",
           handlers::new_reservation_handler(BOT_TOKEN));

  svr.Post("/api/remove_reservation",
           handlers::remove_reservation_handler(BOT_TOKEN));

  svr.Post("/api/get_reservations",
           handlers::get_reservations_handler(BOT_TOKEN));

  std::cout << "Listening on 0.0.0.0:" << PORT << "...\n";
  svr.listen("0.0.0.0", PORT);

  return 0;
}
