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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include "config.h"
#include "telegram.h"

#define POLLING_INTERVAL 10000 // 10 ms

static void trim (char *s) {
        int i = strlen(s) - 1;
        if ((i > 0) && (s[i] == '\n'))
                s[i] = '\0';
}

static const char *get_user_name()
{
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);
        if (pw)
                return pw->pw_name;

        return "";
}

static bool maybe(const char *question)
{
        printf("\n");
        while (1) {
                char buf[128];
                printf("%s (y/n) ", question);
                fflush(stdout);

                buf[sizeof(buf) - 1] = 0;
                if(!fgets(buf, sizeof(buf), stdin))
                        exit(EXIT_FAILURE);

                switch(buf[0]) {
                case 'Y':
                case 'y':
                        return true;
                case 'N':
                case 'n':
                        return false;
                }
        }
}


int main(int argc, char *argv[])
{
        /* get uid */
        uid_t uid = getuid();

        /* Check if config file already exists or not */
        bool has_config = config_exists(uid);
        if (has_config) {
                printf("You already has previously configs with folloing settings:\n");
                config_print(uid);
                if (!maybe("Do you want to update it?"))
                        return 0; /* exit */
        }

        /* Ask for token */
        char token[128];
        token[sizeof(token) - 1] = '\0';

        printf("Please enter your telegram bot token:\n");
        fgets(token, sizeof(token), stdin);
        trim(token);

        /* Send message to telegram  */
        printf("Please type '/start' to your telegram bot\n");

        /* Wait for user enter special security code, we need this step to get the chat_id in telegram */
        printf("Waiting for user type '/start' in telegram bot channel\n");

        char *chat_id = NULL;
        while (1) {
                chat_id = (char *) telegram_fetch_chat_id(token);

                if (chat_id) {
                        printf("\nFind chat_id: %s\n", chat_id);
                        break;
                }

                usleep(POLLING_INTERVAL);
                free(chat_id);
        }

        /* write to config file */
        if (maybe("Do you want to write setting to your config?")) {
                config_write(uid, token, chat_id);

                /* send something to notify user */
                char message[512];
                sprintf(message,
                        "Welcome to use telegram-authenticator, your setup for user %s is done."
                        ,get_user_name());

                telegram_send(token, chat_id, message);
        }
        else
                printf("Skip update configs\n");

        free(chat_id);
        return 0;
}
