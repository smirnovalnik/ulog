/**
  ******************************************************************************
  * @file    ulog.c
  * @author  Alexander Smirnov
  * @brief   Micro logging system
  * @copyright Copyright (C) 2016, 2018, 2020, 2023 Alexander Smirnov
  ******************************************************************************
*/

#include "ulog.h"
#include "ulog_conf.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define ULOG_COLOR_ERR       "\x1b[31m"
#define ULOG_COLOR_WARN      "\x1b[33m"
#define ULOG_COLOR_DEBUG     "\x1b[34m"
#define ULOG_COLOR_TRACE     "\x1b[36m"
#define ULOG_COLOR_RESET     "\x1b[0m"

static char* ulog_level_color[] = \
    {ULOG_COLOR_RESET, ULOG_COLOR_TRACE, ULOG_COLOR_DEBUG, ULOG_COLOR_RESET, ULOG_COLOR_WARN, ULOG_COLOR_ERR};

static char* ulog_level_str[] = \
    {"", "TRACE", "DEBUG", " INFO", " WARN", "  ERR"};

static char ulog_dest_available = ULOG_NULL;
static char ulog_level = ULOG_DEBUG_LVL;

void ulog_init(char dest)
{
    ulog_dest_available = dest;
}

void ulog_deinit(void)
{
    ulog_dest_available = ULOG_NULL;
}

char ulog_get_dest(void)
{
    return ulog_dest_available;
}

void ulog_set_level(char level)
{
    if (level >= ULOG_TRACE_LVL && level <= ULOG_ERR_LVL)
        ulog_level = level;
}

char ulog_get_level(void)
{
    return ulog_level;
}

/**
  * @brief  ulog example: 2000/01/01 06:03:22.000 [  protect] Overcurrent
  */
void ulog(char dest, char level, const char* tag, const char* msg, ...)
{
    if ((dest & ulog_dest_available) == ULOG_NULL)
        return;

    if (level < ulog_level)
        return;

    ULOG_CREATE_MUTEX();
    ULOG_MUTEX_TAKE();

    static va_list args;
    static time_t rawtime;
    static struct tm* timeinfo;
    static char ftime[sizeof("2000/01/01 06:03:22.000 ")];
    #if ULOG_PRINT_TAG == 1
    static char ftag[sizeof("[          ] ")];
    #endif
    static char fmsg[64];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    snprintf(ftime, sizeof(ftime), "%04d/%02d/%02d %02d:%02d:%02d.%03d",
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
            timeinfo->tm_min, timeinfo->tm_sec, 0);

    #if ULOG_PRINT_TAG == 1
    snprintf(ftag, sizeof(ftag), "[%10.10s]", tag);
    #endif

    va_start(args, msg);
    vsnprintf(fmsg, sizeof(fmsg), msg, args);
    va_end(args);

    #if ULOG_REMOVE_SPACES == 1
    /* Delete unnecessary spaces from message: \n, \r, \t*/
    for (char* ch_msg = fmsg; *ch_msg != '\0'; ch_msg++)
        if (*ch_msg == '\n' || *ch_msg == '\r' || *ch_msg == '\t')
            *ch_msg = ' ';
    #endif

    /* Log to stdout */
    if (dest & ulog_dest_available & ULOG_STDOUT)
    {
        /* Print date time*/
        printf("%s ", ftime);

        #if ULOG_PRINT_TAG == 1
        /* Print tag */
        printf("%s ", ftag);
        #endif

        #if ULOG_PRINT_COLOR == 1
        /* Set message color */
        printf("%s", ulog_level_color[level]);
        #endif

        #if ULOG_PRINT_LEVEL == 1
        /* Print lelvel */
        printf("%s ", ulog_level_str[level]);
        #endif

        /* Print message string */
        printf("%s", fmsg);

        #if ULOG_PRINT_COLOR == 1
        /* Reset message color */
        if (level != ULOG_INFO_LVL)
            printf(ULOG_COLOR_RESET);
        #endif

        printf(ULOG_ENDLINE);
    }

    /* Log to file */
    #if ULOG_FILE_SYSTEM == 1
    if (dest & ulog_dest_available & ULOG_FS)
    {
        #if ULOG_USE_POSIX == 1
        int rc = -1;
        FILE *fp;

        /* Check existance */
        if ((fp = fopen(ULOG_FILE_NAME, "r")) != NULL)
        {
            rc = fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            if (rc != -1 && size >= ULOG_MAX_FILE_SIZE)
            {
                remove(ULOG_FILE_NAME".bak");
                rename(ULOG_FILE_NAME, ULOG_FILE_NAME".bak");
            }
            fclose(fp);
        }

        if ((fp = fopen(ULOG_FILE_NAME, "a")) != NULL)
        {
            /* Write date time*/
            rc = fprintf(fp, "%s ", ftime);
            if (rc < 0) goto err;

            #if ULOG_PRINT_TAG == 1
            /* Write tag */
            rc = fprintf(fp, "%s ", ftag);
            if (rc < 0) goto err;
            #endif

            #if ULOG_PRINT_LEVEL == 1
            /* Write level */
            rc = fprintf(fp, "%s ", ulog_level_str[level]);
            if (rc < 0) goto err;
            #endif

            /* Write message string */
            rc = fprintf(fp, "%s", fmsg);
            if (rc < 0) goto err;

            /* Write endline character */
            rc = fprintf(fp, "%s", ULOG_ENDLINE);
            if (rc < 0) goto err;
        }

err:    fclose(fp);
        #else
        FIL f;
        FRESULT rc;
        FILINFO fno;
        UINT bw;

        /* Check existance */
        rc = f_stat(ULOG_FILE_NAME, &fno);
        if (rc == FR_OK && fno.fsize >= LOG_MAX_FILE_SIZE)
        {
            /* Check existance of a backup */
            rc = f_stat(ULOG_FILE_NAME".bak", &fno);
            if (rc == FR_OK)
            {
                f_unlink(ULOG_FILE_NAME".bak");
            }

            f_rename(ULOG_FILE_NAME, ULOG_FILE_NAME".bak");
        }

        rc = f_open(&f, ULOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND);
        if (rc == FR_OK)
        {
            /* Write date time*/
            rc = f_write(&f, ftime, sizeof(ftime) - 1, &bw);
            if (rc) goto err;

            /* Write tag */
            rc = f_write(&f, ftag, sizeof(ftag) - 1, &bw);
            if (rc) goto err;

            /* Write message string */
            rc = f_write(&f, fmsg, strlen(fmsg), &bw);
            if (rc) goto err;

            /* Write endline character */
            rc = f_write(&f, ULOG_ENDLINE, sizeof(ULOG_ENDLINE) - 1, &bw);
            if (rc) goto err;

err:        f_close(&f);
        }
        #endif /* ULOG_USE_POSIX */
    }
    #endif /* ULOG_FILE_SYSTEM */
    ULOG_MUTEX_GIVE();
}
