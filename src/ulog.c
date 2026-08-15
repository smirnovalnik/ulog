/**
  ******************************************************************************
  * @file    ulog.c
  * @author  Alexander Smirnov
  * @brief   Micro logging system
  * @copyright Copyright (C) 2016 - 2026 Alexander Smirnov
  ******************************************************************************
*/

#define ULOG_IMPL

#include "ulog.h"
#include "ulog_conf.h"

#if ULOG_MESSAGE_LEN > 256
#warning "ULOG_MESSAGE_LEN is very large, may cause stack overflow"
#endif

#if ULOG_ENABLE == 1

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define ULOG_COLOR_ERR       "\x1b[31m"
#define ULOG_COLOR_WARN      "\x1b[33m"
#define ULOG_COLOR_DEBUG     "\x1b[34m"
#define ULOG_COLOR_TRACE     "\x1b[36m"
#define ULOG_COLOR_RESET     "\x1b[0m"

static const char* const ulog_level_color[] = \
    {ULOG_COLOR_RESET, ULOG_COLOR_TRACE, ULOG_COLOR_DEBUG, ULOG_COLOR_RESET, ULOG_COLOR_WARN, ULOG_COLOR_ERR};

#if ULOG_LEVEL_SHORT == 1
static const char* const ulog_level_str[] = \
    {"", "T", "D", "I", "W", "E"};
#else
static const char* const ulog_level_str[] = \
    {"", "TRACE", "DEBUG", " INFO", " WARN", "  ERR"};
#endif

static volatile uint8_t ulog_dest_available = ULOG_NULL;
static volatile uint8_t ulog_level = ULOG_DEBUG_LVL;
static ulog_printf_t ulog_func = NULL;

void ulog_init(uint8_t dest)
{
    ulog_dest_available = dest;
    ULOG_CREATE_MUTEX();
}

void ulog_deinit(void)
{
    ulog_dest_available = ULOG_NULL;
}

uint8_t ulog_get_dest(void)
{
    return ulog_dest_available;
}

void ulog_set_level(uint8_t level)
{
    if (level >= ULOG_TRACE_LVL && level <= ULOG_ERR_LVL)
        ulog_level = level;
}

uint8_t ulog_get_level(void)
{
    return ulog_level;
}

void ulog_set_func(ulog_printf_t func)
{
    ulog_func = func;
}

/**
  * @brief  ulog example: 2000/01/01 06:03:22.000 [  protect] Overcurrent
  */
void ulog(uint8_t dest, uint8_t level, const char* tag, const char* msg, ...)
{
    if ((dest & ulog_dest_available) == ULOG_NULL)
        return;

    if (level < ulog_level || level > ULOG_ERR_LVL)
        return;

    if (tag == NULL || msg == NULL)
        return;

    ULOG_MUTEX_TAKE();

    #if ULOG_TIMESTAMP == 1
    struct tm* timeinfo;
    #if ULOG_TIMESTAMP_MS == 1
    char ftime[sizeof("2000/01/01 06:03:22.000")];
    struct timespec rawtime;

    clock_gettime(CLOCK_REALTIME, &rawtime);
    timeinfo = localtime(&rawtime.tv_sec);
    int ms = (int)(rawtime.tv_nsec / 1000000);
    int len = (int)strftime(ftime, sizeof(ftime), "%Y/%m/%d %H:%M:%S", timeinfo);
    if (len > 0 && (size_t)len < sizeof(ftime))
    {
        snprintf(ftime + len, sizeof(ftime) - (size_t)len, ".%03d", ms);
    }
    #else
    char ftime[sizeof("2000/01/01 06:03:22")];
    time_t rawtime;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(ftime, sizeof(ftime), "%Y/%m/%d %H:%M:%S", timeinfo);
    #endif
    #else
    char ftime[sizeof("(0123456789)")];
    unsigned int ticks;

    ticks = (unsigned int)clock();
    snprintf(ftime, sizeof(ftime), "(%010u)", ticks);
    #endif

    #if ULOG_PRINT_TAG == 1
    char ftag[sizeof("[          ]")];
    snprintf(ftag, sizeof(ftag), "[%10.10s]", tag);
    #else
    (void)tag;
    #endif

    va_list args;
    char fmsg[ULOG_MESSAGE_LEN];
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
        #if ULOG_PRINT_COLOR == 1 && ULOG_PRINT_LEVEL == 1 && ULOG_PRINT_TAG == 1
        printf("%s %s %s%s %s%s" ULOG_ENDLINE,
               ftime, ftag, ulog_level_color[level], ulog_level_str[level], fmsg,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_COLOR == 1 && ULOG_PRINT_LEVEL == 1
        printf("%s %s%s %s%s" ULOG_ENDLINE,
               ftime, ulog_level_color[level], ulog_level_str[level], fmsg,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_COLOR == 1 && ULOG_PRINT_TAG == 1
        printf("%s %s %s%s%s" ULOG_ENDLINE,
               ftime, ftag, ulog_level_color[level], fmsg,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_LEVEL == 1 && ULOG_PRINT_TAG == 1
        printf("%s %s %s %s" ULOG_ENDLINE, ftime, ftag, ulog_level_str[level], fmsg);
        #elif ULOG_PRINT_COLOR == 1
        printf("%s %s%s%s" ULOG_ENDLINE,
               ftime, ulog_level_color[level], fmsg,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_LEVEL == 1
        printf("%s %s %s" ULOG_ENDLINE, ftime, ulog_level_str[level], fmsg);
        #elif ULOG_PRINT_TAG == 1
        printf("%s %s %s" ULOG_ENDLINE, ftime, ftag, fmsg);
        #else
        printf("%s %s" ULOG_ENDLINE, ftime, fmsg);
        #endif
    }

    /* Log to custom function */
    if ((dest & ulog_dest_available & ULOG_FUNC) && ulog_func != NULL)
    {
        #if ULOG_PRINT_LEVEL == 1 && ULOG_PRINT_TAG == 1
        ulog_func("%s %s %s %s" ULOG_ENDLINE, ftime, ftag, ulog_level_str[level], fmsg);
        #elif ULOG_PRINT_LEVEL == 1
        ulog_func("%s %s %s" ULOG_ENDLINE, ftime, ulog_level_str[level], fmsg);
        #elif ULOG_PRINT_TAG == 1
        ulog_func("%s %s %s" ULOG_ENDLINE, ftime, ftag, fmsg);
        #else
        ulog_func("%s %s" ULOG_ENDLINE, ftime, fmsg);
        #endif
    }

    /* Log to file */
    #if ULOG_FILE_SYSTEM == 1
    if (dest & ulog_dest_available & ULOG_FS)
    {
        #if ULOG_USE_POSIX_IO == 1
        int rc = -1;
        FILE *fp;

        /* Check existance */
        if ((fp = fopen(ULOG_FILE_NAME, "r")) != NULL)
        {
            rc = fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            if (rc != -1 && size != -1 && size >= ULOG_MAX_FILE_SIZE)
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

err:        fclose(fp);
        }
        #else
        FIL f;
        FRESULT rc;
        FILINFO fno;
        UINT bw;

        /* Check existance */
        rc = f_stat(ULOG_FILE_NAME, &fno);
        if (rc == FR_OK && fno.fsize >= ULOG_MAX_FILE_SIZE)
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
            if (rc != FR_OK) goto err;
            rc = f_write(&f, " ", 1, &bw);
            if (rc != FR_OK) goto err;

            #if ULOG_PRINT_TAG == 1
            /* Write tag */
            rc = f_write(&f, ftag, sizeof(ftag) - 1, &bw);
            if (rc != FR_OK) goto err;
            rc = f_write(&f, " ", 1, &bw);
            if (rc != FR_OK) goto err;
            #endif

            #if ULOG_PRINT_LEVEL == 1
            /* Write level */
            rc = f_write(&f, ulog_level_str[level], strlen(ulog_level_str[level]), &bw);
            if (rc != FR_OK) goto err;
            rc = f_write(&f, " ", 1, &bw);
            if (rc != FR_OK) goto err;
            #endif

            /* Write message string */
            rc = f_write(&f, fmsg, strlen(fmsg), &bw);
            if (rc != FR_OK) goto err;

            /* Write endline character */
            rc = f_write(&f, ULOG_ENDLINE, sizeof(ULOG_ENDLINE) - 1, &bw);
            if (rc != FR_OK) goto err;

err:        f_close(&f);
        }
        #endif /* ULOG_USE_POSIX_IO */
    }
    #endif /* ULOG_FILE_SYSTEM */
    ULOG_MUTEX_GIVE();
}

/**
  * @brief  Dump binary buffer in hexadecimal format
  */
void ulog_dump(uint8_t dest, uint8_t level, const char* tag, const char* desc, const void* data, size_t len)
{
    if ((dest & ulog_dest_available) == ULOG_NULL)
        return;

    if (level < ulog_level || level > ULOG_ERR_LVL)
        return;

    if (tag == NULL || desc == NULL || data == NULL || len == 0)
        return;

    const uint8_t* bytes = (const uint8_t*)data;

    ULOG_MUTEX_TAKE();

    /* Print header with description */
    #if ULOG_TIMESTAMP == 1
    struct tm* timeinfo;
    #if ULOG_TIMESTAMP_MS == 1
    char ftime[sizeof("2000/01/01 06:03:22.000")];
    struct timespec rawtime;

    clock_gettime(CLOCK_REALTIME, &rawtime);
    timeinfo = localtime(&rawtime.tv_sec);
    int ms = (int)(rawtime.tv_nsec / 1000000);
    int len_t = (int)strftime(ftime, sizeof(ftime), "%Y/%m/%d %H:%M:%S", timeinfo);
    if (len_t > 0 && (size_t)len_t < sizeof(ftime))
    {
        snprintf(ftime + len_t, sizeof(ftime) - (size_t)len_t, ".%03d", ms);
    }
    #else
    char ftime[sizeof("2000/01/01 06:03:22")];
    time_t rawtime;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(ftime, sizeof(ftime), "%Y/%m/%d %H:%M:%S", timeinfo);
    #endif
    #else
    char ftime[sizeof("(0123456789)")];
    unsigned int ticks;

    ticks = (unsigned int)clock();
    snprintf(ftime, sizeof(ftime), "(%010u)", ticks);
    #endif

    #if ULOG_PRINT_TAG == 1
    char ftag[sizeof("[          ]")];
    snprintf(ftag, sizeof(ftag), "[%10.10s]", tag);
    #else
    (void)tag;
    #endif

    /* Print description line */
    if (dest & ulog_dest_available & ULOG_STDOUT)
    {
        #if ULOG_PRINT_COLOR == 1 && ULOG_PRINT_LEVEL == 1 && ULOG_PRINT_TAG == 1
        printf("%s %s %s%s %s (%zu bytes)%s" ULOG_ENDLINE,
               ftime, ftag, ulog_level_color[level], ulog_level_str[level], desc, len,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_COLOR == 1 && ULOG_PRINT_LEVEL == 1
        printf("%s %s%s %s (%zu bytes)%s" ULOG_ENDLINE,
               ftime, ulog_level_color[level], ulog_level_str[level], desc, len,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_COLOR == 1 && ULOG_PRINT_TAG == 1
        printf("%s %s %s%s (%zu bytes)%s" ULOG_ENDLINE,
               ftime, ftag, ulog_level_color[level], desc, len,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_LEVEL == 1 && ULOG_PRINT_TAG == 1
        printf("%s %s %s %s (%zu bytes)" ULOG_ENDLINE, ftime, ftag, ulog_level_str[level], desc, len);
        #elif ULOG_PRINT_COLOR == 1
        printf("%s %s%s (%zu bytes)%s" ULOG_ENDLINE,
               ftime, ulog_level_color[level], desc, len,
               (level != ULOG_INFO_LVL) ? ULOG_COLOR_RESET : "");
        #elif ULOG_PRINT_LEVEL == 1
        printf("%s %s %s (%zu bytes)" ULOG_ENDLINE, ftime, ulog_level_str[level], desc, len);
        #elif ULOG_PRINT_TAG == 1
        printf("%s %s %s (%zu bytes)" ULOG_ENDLINE, ftime, ftag, desc, len);
        #else
        printf("%s %s (%zu bytes)" ULOG_ENDLINE, ftime, desc, len);
        #endif
    }

    /* Print description to custom function */
    if ((dest & ulog_dest_available & ULOG_FUNC) && ulog_func != NULL)
    {
        #if ULOG_PRINT_LEVEL == 1 && ULOG_PRINT_TAG == 1
        ulog_func("%s %s %s %s (%zu bytes)" ULOG_ENDLINE, ftime, ftag, ulog_level_str[level], desc, len);
        #elif ULOG_PRINT_LEVEL == 1
        ulog_func("%s %s %s (%zu bytes)" ULOG_ENDLINE, ftime, ulog_level_str[level], desc, len);
        #elif ULOG_PRINT_TAG == 1
        ulog_func("%s %s %s (%zu bytes)" ULOG_ENDLINE, ftime, ftag, desc, len);
        #else
        ulog_func("%s %s (%zu bytes)" ULOG_ENDLINE, ftime, desc, len);
        #endif
    }

    /* Dump binary data line by line */
    for (size_t offset = 0; offset < len; offset += ULOG_DUMP_BYTES_PER_LINE)
    {
        char hex_buf[ULOG_DUMP_BYTES_PER_LINE * 3 + 1];
        char ascii_buf[ULOG_DUMP_BYTES_PER_LINE + 1];
        size_t line_len = (len - offset < ULOG_DUMP_BYTES_PER_LINE) ? (len - offset) : ULOG_DUMP_BYTES_PER_LINE;

        /* Format hex bytes */
        for (size_t i = 0; i < line_len; i++)
        {
            snprintf(hex_buf + i * 3, 4, "%02x ", bytes[offset + i]);
            #if ULOG_DUMP_SHOW_ASCII == 1
            ascii_buf[i] = (bytes[offset + i] >= 32 && bytes[offset + i] <= 126) ? bytes[offset + i] : '.';
            #endif
        }

        /* Pad if line is not full */
        for (size_t i = line_len; i < ULOG_DUMP_BYTES_PER_LINE; i++)
        {
            snprintf(hex_buf + i * 3, 4, "   ");
            #if ULOG_DUMP_SHOW_ASCII == 1
            ascii_buf[i] = ' ';
            #endif
        }

        hex_buf[ULOG_DUMP_BYTES_PER_LINE * 3] = '\0';
        #if ULOG_DUMP_SHOW_ASCII == 1
        ascii_buf[ULOG_DUMP_BYTES_PER_LINE] = '\0';
        #endif

        /* Print to stdout */
        if (dest & ulog_dest_available & ULOG_STDOUT)
        {
            #if ULOG_DUMP_SHOW_ASCII == 1
            printf("  %04zx: %s %s" ULOG_ENDLINE, offset, hex_buf, ascii_buf);
            #else
            printf("  %04zx: %s" ULOG_ENDLINE, offset, hex_buf);
            #endif
        }

        /* Print to custom function */
        if ((dest & ulog_dest_available & ULOG_FUNC) && ulog_func != NULL)
        {
            #if ULOG_DUMP_SHOW_ASCII == 1
            ulog_func("  %04zx: %s %s" ULOG_ENDLINE, offset, hex_buf, ascii_buf);
            #else
            ulog_func("  %04zx: %s" ULOG_ENDLINE, offset, hex_buf);
            #endif
        }

        /* Log to file */
        #if ULOG_FILE_SYSTEM == 1
        if (dest & ulog_dest_available & ULOG_FS)
        {
            #if ULOG_USE_POSIX_IO == 1
            FILE *fp;
            if ((fp = fopen(ULOG_FILE_NAME, "a")) != NULL)
            {
                if (offset == 0)
                {
                    fprintf(fp, "%s ", ftime);
                    #if ULOG_PRINT_TAG == 1
                    fprintf(fp, "%s ", ftag);
                    #endif
                    #if ULOG_PRINT_LEVEL == 1
                    fprintf(fp, "%s ", ulog_level_str[level]);
                    #endif
                    fprintf(fp, "%s (%zu bytes)%s", desc, len, ULOG_ENDLINE);
                }
                #if ULOG_DUMP_SHOW_ASCII == 1
                fprintf(fp, "  %04zx: %s %s%s", offset, hex_buf, ascii_buf, ULOG_ENDLINE);
                #else
                fprintf(fp, "  %04zx: %s%s", offset, hex_buf, ULOG_ENDLINE);
                #endif
                fclose(fp);
            }
            #else
            FIL f;
            FRESULT rc;
            UINT bw;
            char line_buf[128];

            rc = f_open(&f, ULOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND);
            if (rc == FR_OK)
            {
                if (offset == 0)
                {
                    f_write(&f, ftime, strlen(ftime), &bw);
                    f_write(&f, " ", 1, &bw);
                    #if ULOG_PRINT_TAG == 1
                    f_write(&f, ftag, strlen(ftag), &bw);
                    f_write(&f, " ", 1, &bw);
                    #endif
                    #if ULOG_PRINT_LEVEL == 1
                    f_write(&f, ulog_level_str[level], strlen(ulog_level_str[level]), &bw);
                    f_write(&f, " ", 1, &bw);
                    #endif
                    snprintf(line_buf, sizeof(line_buf), "%s (%zu bytes)%s", desc, len, ULOG_ENDLINE);
                    f_write(&f, line_buf, strlen(line_buf), &bw);
                }
                #if ULOG_DUMP_SHOW_ASCII == 1
                snprintf(line_buf, sizeof(line_buf), "  %04zx: %s %s%s", offset, hex_buf, ascii_buf, ULOG_ENDLINE);
                #else
                snprintf(line_buf, sizeof(line_buf), "  %04zx: %s%s", offset, hex_buf, ULOG_ENDLINE);
                #endif
                f_write(&f, line_buf, strlen(line_buf), &bw);
                f_close(&f);
            }
            #endif /* ULOG_USE_POSIX_IO */
        }
        #endif /* ULOG_FILE_SYSTEM */
    }

    ULOG_MUTEX_GIVE();
}

#endif /* ULOG_ENABLE */
