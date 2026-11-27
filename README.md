## Cloning

To clone the project, use `git clone --recurse-submodules https://github.com/erdUha/line_bot.git`

## Requirements on Debian/Ubuntu

To install all needed packages run:

```bash
sudo apt update && sudo apt install -y libssl-dev cmake postgresql gcc-14 g++-14 libpq-dev ninja-build pkg-config ca-certificates
```

## Environment setup

For configuration create `.env` file in the project root directory

Sample `.env` file for native build:

```env
PORT=8080
LINE_BOT_TOKEN=[bot_token]
DATABASE_URL='dbname=line user=[db_user] password=[db_password] host=127.0.0.1 port=5432'
ADMIN_TG_ID=[admin_telegram_id]
ADMIN_USERNAME=[admin_username]
GROUP_PASSWORD='[bcrypt_password_hash]'
```

Sample `.env-docker` file for docker-compose build:

```env
PORT=8080
LINE_BOT_TOKEN=[bot_token]
DATABASE_URL='dbname=line user=[db_user] password=[db_password] host=db port=5432'
POSTGRES_DB=line
POSTGRES_USER=[db_user]
POSTGRES_PASSWORD={db_password]
ADMIN_TG_ID=[admin_telegram_id]
ADMIN_USERNAME=[admin_username]
GROUP_PASSWORD='[bcrypt_password_hash]'
```
### Group password hash generation using bash

Use `htpasswd` command from `apache2-utils` package to automatically append .env file with your group password:

```bash
printf "GROUP_PASSWORD='%s'\n" $(htpasswd -bnBC 10 '' [group_password] | tr -d ':\n') >> .env
```

## Running on Debian/Ubuntu

Run:

```bash
./build.sh
```

## Docker

To run the project in docker mode use `docker-compose`:

```bash
sudo docker compose --env-file .env-docker up --build
```

After .env-docker changes run:

```bash
sudo docker compose --env-file .env-docker build --no-cache
sudo docker compose --env-file .env-docker up
```
