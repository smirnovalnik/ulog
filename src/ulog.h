/**
  ******************************************************************************
  * @file    ulog.h
  * @author  Alexander Smirnov
  * @brief   Micro logging system
  * @copyright Copyright (C) 2016 - 2026 Alexander Smirnov
  ******************************************************************************
*/

#ifndef _ULOG_H_
#define _ULOG_H_

#include <stdint.h>

#include "ulog_conf.h"

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

#if ULOG_ENABLE == 1

/** Initialization */
void ulog_init(uint8_t dest);
/** Deinitialization */
void ulog_deinit(void);
/** Get current destination */
uint8_t ulog_get_dest(void);
/** Set maximum level */
void ulog_set_level(uint8_t level);
/** Get level */
uint8_t ulog_get_level(void);

/**
  * Log a message
  * @brief  System log Ex: 2000/01/01 06:03:22.000 [  protect]   ERR Overcurrent
  */
void ulog(uint8_t dest, uint8_t level, const char* tag, const char* msg, ...);

#else

#define ulog_init(dest)         ((void)0)
#define ulog_deinit()           ((void)0)
#define ulog_get_dest()         (ULOG_NULL)
#define ulog_set_level(level)   ((void)0)
#define ulog_get_level()        (ULOG_NONE_LVL)
#define ulog(dest, level, tag, msg, ...) ((void)0)

#endif /* ULOG_ENABLE */

/** Short macros */
#define ULOG_TRACE(tag, ...) ulog(ULOG_STDOUT | ULOG_FS, ULOG_TRACE_LVL, tag, __VA_ARGS__)
#define ULOG_DEBUG(tag, ...) ulog(ULOG_STDOUT | ULOG_FS, ULOG_DEBUG_LVL, tag, __VA_ARGS__)
#define ULOG_INFO(tag, ...)  ulog(ULOG_STDOUT | ULOG_FS, ULOG_INFO_LVL, tag, __VA_ARGS__)
#define ULOG_WARN(tag, ...)  ulog(ULOG_STDOUT | ULOG_FS, ULOG_WARN_LVL, tag, __VA_ARGS__)
#define ULOG_ERR(tag, ...)   ulog(ULOG_STDOUT | ULOG_FS, ULOG_ERR_LVL, tag, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */
