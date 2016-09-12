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

#ifndef _TELEGRAM_AUTHENTICATOR_CONFIG_H_
#define _TELEGRAM_AUTHENTICATOR_CONFIG_H_

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
        char *token;            /* telegram bot token */
        char *chat_id;          /* telegram chat_id */
} config_t;


/**
 * Check if user's config file exists or not, the config can find at ~/.telegram_authenticator
 *
 * @param uid   user uid to find user home dir
 *
 * @return  false  config not exist
 *          true   config exist
 */
bool config_exists(uid_t uid);

/**
 * Return user's config file path.
 * The returned value should be freed when no longer needed.
 *
 * @param uid   user uid to find user home dir
 *
 * @return config file path
 */
const char *config_file(uid_t uid);

/**
 * Update user's config file.
 * This function will write config in json format to ~/.telegram_authenticator.
 *
 * @param uid   user uid to find user home dir
 *
 * @param token     telegram bot's token
 * @param chat_id   telegram room chat_id
 *
 */
void config_write(uid_t uid, const char *token, const char *chat_id);

/**
 * Read usre's config file, return in config_t struct.
 * The returned value should use config_free() when no longer needed.
 *
 * @param uid   user uid to find user home dir
 *
 * @return config_t  file exist and config can parse
 *         NULL      file not exist
 */
config_t config_read(uid_t uid);

/**
 * Free the config_t structure.
 *
 * @param cfg
 */
void config_free(config_t cfg);


/**
 * Print the configuration values to stdout.
 *
 * @param uid   user uid to find user home dir
 *
 */
void config_print(uid_t uid);


#endif /* _TELEGRAM_AUTHENTICATOR_CONFIG_H_ */