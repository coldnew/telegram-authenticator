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

#include "telegram.h"

#include <curl/curl.h>
#include <json-c/json.h>

#define BOT_API_URL "https://api.telegram.org/bot"

typedef struct {
        char *text;
        size_t size;
} json_message;


/**
 * Return telegram's bot api url according to key.
 * The returned value should be freed when no longer needed.
 *
 * @param token   telegarm bot token
 * @param method  telegram bot method
 *
 * @return telegram bot api url
 */
static
const char *telegram_api_url(const char *token, const char *method)
{
        if ((!token) || (!method)) {
                fprintf(stderr, "ERROR: bot token should not be NULL\n");
                exit(EXIT_FAILURE);
        }

        char *url = malloc(strlen(BOT_API_URL) + strlen(token) + strlen(method) + 1);
        if (!url) {
                perror("malloc()");
                exit(EXIT_FAILURE);
        }

        return strcat(strcat(strcpy(url, BOT_API_URL), token), method);
}

/**
 * Return telegram's bot getUpdates api rul
 * The returned value should be freed when no longer needed.
 *
 * @param token   telegarm bot token
 *
 * @return telegram bot getUpdates api url
 */
static inline
const char *telegram_api_getUpdates(const char *token)
{
        return telegram_api_url(token, "/getUpdates");
}

/**
 * Return telegram's bot sendMessage api rul
 * The returned value should be freed when no longer needed.
 *
 * @param token   telegarm bot token
 *
 * @return telegram bot sendMessage api url
 */
static inline
const char *telegram_api_sendMessage(const char *token)
{
        return telegram_api_url(token, "/sendMessage");
}

/**
 * Setup curl's headers to make it send with json request.
 *
 * @param curl curl handler
 */
static inline
void curl_set_json_headers(CURL *curl)
{
        struct curl_slist *headers = NULL;

        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
}

/* Callback for make curl not show response */
static
size_t dummy_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
        return size * nmemb;
}

/**
 * Send message to telegram channel.
 *
 * @param token     telegram bot token
 * @param chat_id   telegram chat channel id
 * @param msg       message send to telegram
 *
 * @return  false   failed to send message
 *          true    send message success
 */
bool telegram_send(const char *token, const char *chat_id, const char *msg)
{
        const char *url = telegram_api_sendMessage(token);
        CURLcode res;

        CURL *curl = curl_easy_init();
        if (!curl) {
                fprintf(stderr, "ERROR: Failed on curl_easy_init().");
                return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_set_json_headers(curl);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_callback); /* ignore response callback */

        /* build request data */
        json_object *jobj = json_object_new_object();
        json_object *jval;

        jval = json_object_new_string(chat_id);
        json_object_object_add(jobj, "chat_id", jval);

        jval = json_object_new_string(msg);
        json_object_object_add(jobj, "text", jval);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(jobj));

        res = curl_easy_perform(curl);

        /* check return code */
        if (CURLE_OK != res) {
                fprintf(stderr, "ERROR: Failed to send to url (%s) - curl said: %s", url, curl_easy_strerror(res));
                return false;
        }

        curl_easy_cleanup(curl);
        json_object_put(jobj);  /* free json object */

        free((char *) url);

        return true;
}


/* Callback for curl read func */
static
size_t write_callback(void *buffer, size_t size, size_t nmemb, void *dest) {
        size_t rsize = size * nmemb;
        json_message *message = (json_message *) dest;

        message->text = realloc(message->text, message->size + rsize + 1);

        if (!message->text) {
                fprintf(stderr, "ERROR: no message readed from telegram channel!!!");
                return 0;
        }

        memcpy(&(message->text[message->size]), buffer, rsize);
        message->size += rsize;
        message->text[message->size] = '\0';

        return rsize;
}

/**
 * Start a long polling loop wait for user input specific keyword, after input match, return channel's chat_id.
 * The returned value should be freed when no longer needed.
 *
 * @param token telegram bot token
 *
 * @return chat_id  telegram bot chat_id
 *         NULL     failed to fetch chat_id (user not input ?)
 */
const char *telegram_fetch_chat_id(const char *token)
{
        char *chat_id = NULL;

        const char *url = telegram_api_getUpdates(token);

        json_message rdata = {0, 0}; /* data we read */
        rdata.text = (char*) malloc(1 * sizeof(char));
        rdata.text[0] = '\0';

        /* setup curl handler */
        CURLcode res;
        CURL *curl = curl_easy_init();
        if (!curl) {
                fprintf(stderr, "ERROR: Failed on curl_easy_init().");
                return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rdata);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

        res = curl_easy_perform(curl);
        if (CURLE_OK != res) {
                fprintf(stderr, "ERROR: Failed to send to url (%s) - curl said: %s", url, curl_easy_strerror(res));
                exit(EXIT_FAILURE);
        }

        json_object *root = json_tokener_parse(rdata.text);
        if (is_error(root)) {
                //fprintf(stderr, "parse failed.\n");
                return NULL;
        }

        json_object *body;
        json_object_object_get_ex(root, "result", &body);

        int arraylen = json_object_array_length(body);

        json_object *jvalue;
        for (int i = 0; i < arraylen; i++) {
                jvalue = json_object_array_get_idx(body, i);

                /* we only find the latest match value */
                struct json_object *jmessage;
                json_object_object_get_ex(jvalue, "message", &jmessage);

                struct json_object *jchat;
                json_object_object_get_ex(jmessage, "chat", &jchat);

                struct json_object *jchat_id;
                json_object_object_get_ex(jchat, "id", &jchat_id);

                struct json_object *jtext;
                json_object_object_get_ex(jmessage, "text", &jtext);

                /* We need to make sure the text user type is match to our keyword */
                if (!strcmp(json_object_to_json_string(jtext), "\"\\/start\"")) {
                        chat_id = (char *) json_object_to_json_string(jchat_id);
                }
        }

        curl_easy_cleanup(curl);
        free((char *) url);

        return chat_id;
}