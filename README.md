## Cloning

To clone the project, use `git clone --recurse-submodules https://github.com/erdUha/line_bot.git`

## Requirements on Debian/Ubuntu

`sudo apt update && sudo apt install libssl-dev cmake postgresql gcc-14 g++-14 libpq-dev pkg-config`

## Environment setup

Sample `.env` file:

```env
LINE_BOT_TOKEN=[bot_token]
DATABASE_URL='dbname=line user=[db_user] password=[db_password] host=127.0.0.1 port=5432'
ADMIN_TG_ID=[admin_telegram_id]
ADMIN_USERNAME=[admin_username]
GROUP_PASSWORD='[bcrypt_password_hash]'
```
