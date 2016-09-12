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
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "telegram.h"

#include <security/pam_modules.h>
#include <security/pam_ext.h>

static
uid_t get_user_uid(pam_handle_t* pamh)
{
    /* first we try to get username */
    const char *username;
    pam_get_user(pamh, &username, NULL);

    /*  FIXME: error handling */

    /* get user uid by getpwname_r() */
    size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 4096;

    char *buf = malloc(bufsize);
    if (buf == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    struct passwd pwd, *result;
    int s = getpwnam_r(username, &pwd, buf, bufsize, &result);

    if (result == NULL) {
        if (s == 0)
            printf("Not found\n");
        else {
            perror("getpwnam_r");
            exit(EXIT_FAILURE);
        }
    }

    free(buf);
    return pwd.pw_uid;
}

static
void passwdgen(char *str)
{
    size_t size = 6;
    const char charset[] = "0123456789";
    srand(time(NULL));

    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t* pamh, int flags, int argc,
                                   char const** argv) {

    /* step 1: get user home */
    uid_t uid = get_user_uid(pamh);
    bool has_config = config_exists(uid);
    if (!has_config) {
        pam_syslog(pamh, LOG_NOTICE, "No telegram-authenticator config find, skipped.");
        return PAM_IGNORE;
    }

    /* generate password */
    char passwd[6];
    passwdgen(passwd);

    /* Send password to telegram */
    char msg[128];
    sprintf(msg, "Your ssh login code: %s", passwd);

    config_t cfg = config_read(uid);
    telegram_send(cfg.token, cfg.chat_id, msg);

    char *response;
    int rc = pam_prompt(pamh, PAM_PROMPT_ECHO_OFF, &response, "Telegram Verification: ");

    if (response == NULL)
        rc = PAM_CONV_ERR;

    if (rc != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_WARNING, "No response to query telegram verification code.");
        return rc;
    }

    if (!strcmp(response, passwd))
        return PAM_SUCCESS;

    return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t* pamh, int flags, int argc,
                              char const** argv) {
    return PAM_SUCCESS;
}
