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

/* Block output to stdout, a port can override it in ulog_conf.h to bypass stdio */
#ifndef ULOG_STDOUT_WRITE
#define ULOG_STDOUT_WRITE(buf, len) fwrite(buf, 1, len, stdout)
#endif

static const char* const ulog_level_color[] = \
    {ULOG_COLOR_RESET, ULOG_COLOR_TRACE, ULOG_COLOR_DEBUG, ULOG_COLOR_RESET, ULOG_COLOR_WARN, ULOG_COLOR_ERR};

#if ULOG_LEVEL_SHORT == 1
static const char* const ulog_level_str[] = \
    {"", "T", "D", "I", "W", "E"};
#else
static const char* const ulog_level_str[] = \
    {"", "TRACE", "DEBUG", " INFO", " WARN", "  ERR"};
#endif

static const char ulog_hex[] = "0123456789abcdef";

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

/* Maximum length of the line head: time, tag, color and level */
#define ULOG_HEAD_LEN sizeof("2000/01/01 06:03:22.000 [          ] " ULOG_COLOR_TRACE "TRACE ")

/**
  * @brief  Write the line head "time [tag] color level " to the buffer of ULOG_HEAD_LEN bytes
  * @param  head_len Length of "time [tag] ", the part before the color
  * @param  body Position after the color
  * @return Position after the head
  */
static char* ulog_head(char* line, uint8_t level, const char* tag, int* head_len, char** body)
{
    char* p = line;
    size_t n;

    #if ULOG_TIMESTAMP == 1
    /* Date and time are formatted once per second and kept between the calls, the callers hold the mutex */
    static char ftime[sizeof("2000/01/01 06:03:22")];
    static size_t ftime_len = 0;
    static time_t ftime_sec;
    #if ULOG_TIMESTAMP_MS == 1
    struct timespec rawtime;

    clock_gettime(CLOCK_REALTIME, &rawtime);
    time_t sec = rawtime.tv_sec;
    #else
    time_t sec;

    time(&sec);
    #endif

    if (ftime_len == 0 || sec != ftime_sec)
    {
        ftime_len = strftime(ftime, sizeof(ftime), "%Y/%m/%d %H:%M:%S", localtime(&sec));
        ftime_sec = sec;
    }
    n = ftime_len;
    #else
    char ftime[sizeof("(0123456789)")];
    unsigned int ticks;

    ticks = (unsigned int)clock();
    snprintf(ftime, sizeof(ftime), "(%010u)", ticks);
    n = strlen(ftime);
    #endif

    memcpy(p, ftime, n);
    p += n;
    #if ULOG_TIMESTAMP == 1 && ULOG_TIMESTAMP_MS == 1
    unsigned int ms = (unsigned int)(rawtime.tv_nsec / 1000000) % 1000;
    *p++ = '.';
    *p++ = (char)('0' + ms / 100);
    *p++ = (char)('0' + ms / 10 % 10);
    *p++ = (char)('0' + ms % 10);
    #endif
    *p++ = ' ';

    #if ULOG_PRINT_TAG == 1
    /* Tag is right-aligned in 10 characters, a longer one is cut */
    n = 0;
    while (n < 10 && tag[n] != '\0')
        n++;
    *p++ = '[';
    memset(p, ' ', 10 - n);
    memcpy(p + 10 - n, tag, n);
    p += 10;
    *p++ = ']';
    *p++ = ' ';
    #else
    (void)tag;
    #endif
    *head_len = (int)(p - line);

    #if ULOG_PRINT_COLOR == 1
    n = strlen(ulog_level_color[level]);
    memcpy(p, ulog_level_color[level], n);
    p += n;
    #endif
    *body = p;

    #if ULOG_PRINT_LEVEL == 1
    n = strlen(ulog_level_str[level]);
    memcpy(p, ulog_level_str[level], n);
    p += n;
    *p++ = ' ';
    #endif

    return p;
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

    /* The whole line is built once: head, color, body (level and message), color reset, endline.
       Stdout gets it as is, the custom function gets head and body without the color. */
    char line[ULOG_HEAD_LEN + ULOG_MESSAGE_LEN + sizeof(ULOG_COLOR_RESET) + sizeof(ULOG_ENDLINE)];
    int head_len;
    char* body;
    char* p = ulog_head(line, level, tag, &head_len, &body);

    va_list args;
    char* fmsg = p;
    va_start(args, msg);
    vsnprintf(fmsg, ULOG_MESSAGE_LEN, msg, args);
    va_end(args);

    #if ULOG_REMOVE_SPACES == 1
    /* Delete unnecessary spaces from message: \n, \r, \t*/
    for (char* ch_msg = fmsg; *ch_msg != '\0'; ch_msg++)
        if (*ch_msg == '\n' || *ch_msg == '\r' || *ch_msg == '\t')
            *ch_msg = ' ';
    #endif

    p += strlen(fmsg);
    int body_len = (int)(p - body);

    #if ULOG_PRINT_COLOR == 1
    if (level != ULOG_INFO_LVL)
    {
        memcpy(p, ULOG_COLOR_RESET, sizeof(ULOG_COLOR_RESET) - 1);
        p += sizeof(ULOG_COLOR_RESET) - 1;
    }
    #endif

    memcpy(p, ULOG_ENDLINE, sizeof(ULOG_ENDLINE) - 1);
    p += sizeof(ULOG_ENDLINE) - 1;

    /* Log to stdout */
    if (dest & ulog_dest_available & ULOG_STDOUT)
    {
        ULOG_STDOUT_WRITE(line, (size_t)(p - line));
    }

    /* Log to custom function */
    if ((dest & ulog_dest_available & ULOG_FUNC) && ulog_func != NULL)
    {
        ulog_func("%.*s%.*s" ULOG_ENDLINE, head_len, line, body_len, body);
    }

    /* Log to file */
    #if ULOG_FILE_SYSTEM == 1
    if (dest & ulog_dest_available & ULOG_FS)
    {
        /* The color is not written to the file: move the body over it */
        memmove(line + head_len, body, (size_t)body_len);
        p = line + head_len + body_len;
        memcpy(p, ULOG_ENDLINE, sizeof(ULOG_ENDLINE) - 1);
        p += sizeof(ULOG_ENDLINE) - 1;

        #if ULOG_USE_POSIX_IO == 1
        FILE *fp = fopen(ULOG_FILE_NAME, "a");

        /* Rotate the full file */
        if (fp != NULL && fseek(fp, 0, SEEK_END) == 0 && ftell(fp) >= ULOG_MAX_FILE_SIZE)
        {
            fclose(fp);
            remove(ULOG_FILE_NAME".bak");
            rename(ULOG_FILE_NAME, ULOG_FILE_NAME".bak");
            fp = fopen(ULOG_FILE_NAME, "a");
        }

        if (fp != NULL)
        {
            fwrite(line, 1, (size_t)(p - line), fp);
            fclose(fp);
        }
        #else
        FIL f;
        FRESULT rc;
        UINT bw;

        rc = f_open(&f, ULOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND);

        /* Rotate the full file */
        if (rc == FR_OK && f_size(&f) >= ULOG_MAX_FILE_SIZE)
        {
            f_close(&f);
            f_unlink(ULOG_FILE_NAME".bak");
            f_rename(ULOG_FILE_NAME, ULOG_FILE_NAME".bak");
            rc = f_open(&f, ULOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND);
        }

        if (rc == FR_OK)
        {
            f_write(&f, line, (UINT)(p - line), &bw);
            f_close(&f);
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

    /* Description line: head and level from the buffer, description as is,
       then the tail " (N bytes)" with color reset and endline */
    char line[ULOG_HEAD_LEN];
    int head_len;
    char* body;
    char* p = ulog_head(line, level, tag, &head_len, &body);
    int level_len = (int)(p - body);

    char tail[sizeof(" (18446744073709551615 bytes)") + sizeof(ULOG_COLOR_RESET) + sizeof(ULOG_ENDLINE)];
    int bytes_len = snprintf(tail, sizeof(tail), " (%lu bytes)", (unsigned long)len);
    char* t = tail + bytes_len;

    #if ULOG_PRINT_COLOR == 1
    if (level != ULOG_INFO_LVL)
    {
        memcpy(t, ULOG_COLOR_RESET, sizeof(ULOG_COLOR_RESET) - 1);
        t += sizeof(ULOG_COLOR_RESET) - 1;
    }
    #endif

    memcpy(t, ULOG_ENDLINE, sizeof(ULOG_ENDLINE) - 1);
    t += sizeof(ULOG_ENDLINE) - 1;

    /* Print description line */
    if (dest & ulog_dest_available & ULOG_STDOUT)
    {
        ULOG_STDOUT_WRITE(line, (size_t)(p - line));
        ULOG_STDOUT_WRITE(desc, strlen(desc));
        ULOG_STDOUT_WRITE(tail, (size_t)(t - tail));
    }

    /* Print description to custom function */
    if ((dest & ulog_dest_available & ULOG_FUNC) && ulog_func != NULL)
    {
        ulog_func("%.*s%.*s%s%.*s" ULOG_ENDLINE, head_len, line, level_len, body, desc, bytes_len, tail);
    }

    /* Print description to file, it stays open for the whole dump */
    #if ULOG_FILE_SYSTEM == 1
    #if ULOG_USE_POSIX_IO == 1
    FILE *fp = NULL;

    if (dest & ulog_dest_available & ULOG_FS)
        fp = fopen(ULOG_FILE_NAME, "a");

    if (fp != NULL)
    {
        fprintf(fp, "%.*s%.*s%s%.*s" ULOG_ENDLINE, head_len, line, level_len, body, desc, bytes_len, tail);
    }
    #else
    FIL f;
    UINT bw;
    int f_opened = (dest & ulog_dest_available & ULOG_FS) &&
                   f_open(&f, ULOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND) == FR_OK;

    if (f_opened)
    {
        f_write(&f, line, (UINT)head_len, &bw);
        f_write(&f, body, (UINT)level_len, &bw);
        f_write(&f, desc, strlen(desc), &bw);
        f_write(&f, tail, (UINT)bytes_len, &bw);
        f_write(&f, ULOG_ENDLINE, sizeof(ULOG_ENDLINE) - 1, &bw);
    }
    #endif /* ULOG_USE_POSIX_IO */
    #endif /* ULOG_FILE_SYSTEM */

    /* Dump binary data line by line */
    for (size_t offset = 0; offset < len; offset += ULOG_DUMP_BYTES_PER_LINE)
    {
        /* The whole line in one buffer: "  offset: hex bytes [ascii]" */
        char dline[sizeof("  : ") + 2 * sizeof(size_t) + ULOG_DUMP_BYTES_PER_LINE * 3 + 1 +
                   ULOG_DUMP_BYTES_PER_LINE + sizeof(ULOG_ENDLINE)];
        char* d = dline;
        size_t line_len = (len - offset < ULOG_DUMP_BYTES_PER_LINE) ? (len - offset) : ULOG_DUMP_BYTES_PER_LINE;

        /* Format offset: 4 hex digits at least */
        unsigned int digits = 4;
        while (digits < 2 * sizeof(size_t) && (offset >> (4 * digits)) != 0)
            digits++;

        *d++ = ' ';
        *d++ = ' ';
        while (digits-- > 0)
            *d++ = ulog_hex[(offset >> (4 * digits)) & 0x0f];
        *d++ = ':';
        *d++ = ' ';

        char* hex_buf = d;
        d += ULOG_DUMP_BYTES_PER_LINE * 3;
        #if ULOG_DUMP_SHOW_ASCII == 1
        *d++ = ' ';
        char* ascii_buf = d;
        d += ULOG_DUMP_BYTES_PER_LINE;
        #endif

        /* Format hex bytes */
        for (size_t i = 0; i < line_len; i++)
        {
            hex_buf[i * 3] = ulog_hex[bytes[offset + i] >> 4];
            hex_buf[i * 3 + 1] = ulog_hex[bytes[offset + i] & 0x0f];
            hex_buf[i * 3 + 2] = ' ';
            #if ULOG_DUMP_SHOW_ASCII == 1
            ascii_buf[i] = (bytes[offset + i] >= 32 && bytes[offset + i] <= 126) ? bytes[offset + i] : '.';
            #endif
        }

        /* Pad if line is not full */
        for (size_t i = line_len; i < ULOG_DUMP_BYTES_PER_LINE; i++)
        {
            memcpy(hex_buf + i * 3, "   ", 3);
            #if ULOG_DUMP_SHOW_ASCII == 1
            ascii_buf[i] = ' ';
            #endif
        }

        memcpy(d, ULOG_ENDLINE, sizeof(ULOG_ENDLINE));
        d += sizeof(ULOG_ENDLINE) - 1;

        /* Print to stdout */
        if (dest & ulog_dest_available & ULOG_STDOUT)
        {
            ULOG_STDOUT_WRITE(dline, (size_t)(d - dline));
        }

        /* Print to custom function */
        if ((dest & ulog_dest_available & ULOG_FUNC) && ulog_func != NULL)
        {
            ulog_func("%s", dline);
        }

        /* Log to file */
        #if ULOG_FILE_SYSTEM == 1
        #if ULOG_USE_POSIX_IO == 1
        if (fp != NULL)
        {
            fwrite(dline, 1, (size_t)(d - dline), fp);
        }
        #else
        if (f_opened)
        {
            f_write(&f, dline, (UINT)(d - dline), &bw);
        }
        #endif /* ULOG_USE_POSIX_IO */
        #endif /* ULOG_FILE_SYSTEM */
    }

    #if ULOG_FILE_SYSTEM == 1
    #if ULOG_USE_POSIX_IO == 1
    if (fp != NULL)
    {
        fclose(fp);
    }
    #else
    if (f_opened)
    {
        f_close(&f);
    }
    #endif /* ULOG_USE_POSIX_IO */
    #endif /* ULOG_FILE_SYSTEM */

    ULOG_MUTEX_GIVE();
}

#endif /* ULOG_ENABLE */
