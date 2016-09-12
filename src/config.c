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
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <pwd.h>

#include "config.h"
#include <json-c/json.h>

#define CONFIG_FILE "/.telegram_authenticator"

/**
 * Remove double quote
 *
 * json_object_to_json_string() will return value with double quote, we need to strip it.
 *
 * @param str
 */
static
char *remove_double_quote(char *str)
{
        char *pr = str, *pw = str;
        while (*pr) {
                *pw = *pr++;
                pw += (*pw != '"');
        }
        *pw = '\0';
        return str;
}

/**
 * Return user's config file path.
 * The returned value should be freed when no longer needed.
 *
 * @return config file path
 */
const char *config_file(uid_t uid)
{
        /* Find config file at user's home dir */
        const char *home = getenv("HOME");
        if (!home || '/' != *home) {
                home = getpwuid(uid)->pw_dir;
                if (!home || '/' != *home) {
                        fprintf(stderr, "Cannot determine user's home directory\n");
                        exit(EXIT_FAILURE);
                }
        }

        char *config = malloc(strlen(home) + strlen(CONFIG_FILE) + 1);
        if (!config) {
                perror("malloc()");
                exit(EXIT_FAILURE);
        }

        return strcat(strcpy(config, home), CONFIG_FILE);
}


/**
 * Check if user's config file exists or not, the config can be finded at ~/.telegram_authenticator
 *
 * @return  false  config not exist
 *          true   config exist
 */
bool config_exists(uid_t uid)
{
        const char *config = config_file(uid);
        struct stat st;

        bool ret = false;
        if (0 == stat(config, &st))
                ret = true;

        free((char *)config);
        return ret;
}


/**
 * Update user's config file.
 * This function will write config in json format to ~/.telegram_authenticator.
 *
 * @param token     telegram bot's token
 * @param chat_id   telegram room chat_id
 *
 */
void config_write(uid_t uid, const char *token, const char *chat_id)
{
        config_t config = config_read(uid);

        json_object *jobj = json_object_new_object();
        json_object *jval;

        if (token)
                config.token = (char *) token;

        if(chat_id)
                config.chat_id = (char *) chat_id;

        jval = json_object_new_string(config.token);
        json_object_object_add(jobj, "token", jval);

        jval = json_object_new_string(config.chat_id);
        json_object_object_add(jobj, "chat_id", jval);

        const char *filename = config_file(uid);
        FILE *f = fopen(filename, "w");
        if (NULL != f)
                fputs(json_object_to_json_string(jobj), f);

        fclose(f);
        free((char *)filename);
        json_object_put(jobj);  /* free json object */
}

/**
 * Read usre's config file, return in config_t struct.
 * The returned value should use config_free() when no longer needed.
 *
 * @return config_t  file exist and config can parse
 *         NULL      file not exist
 */
config_t config_read(uid_t uid)
{
        config_t conf;
        char *buf;

        if (!config_exists(uid)) return conf;

        const char *config = config_file(uid);
        FILE *f = fopen(config, "r");
        if (NULL != f) {
                /* get file size */
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                rewind(f);

                buf = malloc(fsize + 1);
                fread(buf, fsize, 1, f);
        }
        fclose(f);
        free((char *)config);

        /* parse json value */
        struct json_object *root = json_tokener_parse(buf);
        struct json_object *jobj;

        json_object_object_get_ex(root, "token", &jobj);
        conf.token = remove_double_quote((char *) json_object_to_json_string(jobj));

        json_object_object_get_ex(root, "chat_id", &jobj);
        conf.chat_id = remove_double_quote((char *) json_object_to_json_string(jobj));

        free(buf);
        return conf;
}

/**
 * Free the config_t structure.
 *
 * @param cfg
 */
void config_free(config_t cfg)
{
        if (cfg.token)   free(cfg.token);
        if (cfg.chat_id) free(cfg.chat_id);
}

/**
 * Print the configuration values to stdout.
 *
 */
void config_print(uid_t uid)
{
        config_t cfg = config_read(uid);

        printf("\n"
               "\t Bot Token: %s\n"
               "\t chat_id:   %s\n"
               "\n", cfg.token, cfg.chat_id);

        config_free(cfg);
}