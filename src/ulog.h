/**
  ******************************************************************************
  * @file    ulog.h
  * @author  Alexander Smirnov
  * @brief   Micro logging system
  * @copyright Copyright (C) 2016 - 2024 Alexander Smirnov
  ******************************************************************************
*/

#ifndef _ULOG_H_
#define _ULOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/** ulog destination */
enum {
    ULOG_NULL    = 0x00,    /**< Log to null */
    ULOG_STDOUT  = 0x01,    /**< Log to stdout */
    ULOG_FS      = 0x02,    /**< Log to file system */
};

/** ulog level */
enum {
    ULOG_NONE_LVL    = 0x00,
    ULOG_TRACE_LVL   = 0x01,    /**< trace */
    ULOG_DEBUG_LVL   = 0x02,    /**< debug */
    ULOG_INFO_LVL    = 0x03,    /**< information */
    ULOG_WARN_LVL    = 0x04,    /**< warning */
    ULOG_ERR_LVL     = 0x05,    /**< error */
};

/** Initialization */
void ulog_init(char dest);
/** Deinitialization */
void ulog_deinit(void);
/** Get current destination */
char ulog_get_dest(void);
/** Set maximum level */
void ulog_set_level(char level);
/** Get level */
char ulog_get_level(void);

/**
  * Log a message
  * @brief  System log Ex: 2000/01/01 06:03:22.000 [  protect]   ERR Overcurrent
  */
void ulog(unsigned char dest, unsigned char level, const char* tag, const char* msg, ...);

/** Short macros */
#define ULOG_TRACE(tag, msg, ...) ulog(ULOG_STDOUT | ULOG_FS, ULOG_TRACE_LVL, tag, msg, ##__VA_ARGS__)
#define ULOG_DEBUG(tag, msg, ...) ulog(ULOG_STDOUT | ULOG_FS, ULOG_DEBUG_LVL, tag, msg, ##__VA_ARGS__)
#define ULOG_INFO(tag, msg, ...)  ulog(ULOG_STDOUT | ULOG_FS, ULOG_INFO_LVL, tag, msg, ##__VA_ARGS__)
#define ULOG_WARN(tag, msg, ...)  ulog(ULOG_STDOUT | ULOG_FS, ULOG_WARN_LVL, tag, msg, ##__VA_ARGS__)
#define ULOG_ERR(tag, msg, ...)   ulog(ULOG_STDOUT | ULOG_FS, ULOG_ERR_LVL, tag, msg, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */
