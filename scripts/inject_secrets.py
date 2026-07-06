# This script is used to inject secrets into the build process for PlatformIO.

import os
from pathlib import Path

Import("env")


def load_env_file(env_path):
    values = {}

    if not env_path.is_file():
        return values

    with env_path.open("r", encoding="utf-8") as handle:
        for raw_line in handle:
            line = raw_line.strip()

            if not line or line.startswith("#") or "=" not in line:
                continue

            key, value = line.split("=", 1)
            key = key.strip()
            value = value.strip().strip('"').strip("'")

            if key:
                values[key] = value

    return values


project_root = Path(env.subst("$PROJECT_DIR"))
project_env = load_env_file(project_root / ".env")


def get_secret(name):
    return project_env.get(name) or os.getenv(name)


telegram_bot_token = get_secret("TELEGRAM_BOT_TOKEN")
telegram_chat_id = get_secret("TELEGRAM_CHAT_ID")

if telegram_bot_token:
    env.Append(CPPDEFINES=[("telegramBotToken", f'\\"{telegram_bot_token}\\"')])

if telegram_chat_id:
    env.Append(CPPDEFINES=[("telegramChatId", f'\\"{telegram_chat_id}\\"')])

ota_password = get_secret("OTA_PASSWORD")

if ota_password:
    env.Append(UPLOAD_FLAGS=[f"--auth={ota_password}"])