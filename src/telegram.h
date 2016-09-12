/*
 * Copyright (c) 2016 Yen-Chin, Lee <coldnew.tw at gmail.com>.
 *
 * This file is part of telegram-authenticator.
 *
 * telegram-authenticator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * telegram-authenticator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with telegram-authenticator.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _TELEGRAM_AUTHENTICATOR_TELEGRAM_H_
#define _TELEGRAM_AUTHENTICATOR_TELEGRAM_H_

#include <stdbool.h>

/**
 * Send message to telegram channel.
 *
 * @param token     telegram bot token
 * @param chat_id   telegram chat channel id
 * @param msg       message send to telegram
 * @return  false   failed to send message
 *          true    send message success
 */
bool telegram_send(const char *token, const char *chat_id, const char *msg);

/**
 * Wait for user input specific keyword, after input match, return channel's chat_id.
 * You need to use this function inside a loop.
 *
 * The returned value should be freed when no longer needed.
 *
 * @param token telegram bot token
 *
  * @return chat_id  telegram bot chat_id
 *          NULL     failed to fetch chat_id (user not input ?)
 */
const char *telegram_fetch_chat_id(const char *token);

#endif /* _TELEGRAM_AUTHENTICATOR_TELEGRAM_H_ */