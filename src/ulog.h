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
#include <stddef.h>

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

/**
  * Dump binary buffer in hexadecimal format
  * @param  dest Destination (ULOG_STDOUT, ULOG_FS, or combination)
  * @param  level Log level
  * @param  tag Tag string
  * @param  desc Description of the buffer
  * @param  data Pointer to binary data
  * @param  len Length of data in bytes
  */
void ulog_dump(uint8_t dest, uint8_t level, const char* tag, const char* desc, const void* data, size_t len);

/** Short macros */
#define ULOG_TRACE(tag, ...) ulog(ULOG_STDOUT | ULOG_FS, ULOG_TRACE_LVL, tag, __VA_ARGS__)
#define ULOG_DEBUG(tag, ...) ulog(ULOG_STDOUT | ULOG_FS, ULOG_DEBUG_LVL, tag, __VA_ARGS__)
#define ULOG_INFO(tag, ...)  ulog(ULOG_STDOUT | ULOG_FS, ULOG_INFO_LVL, tag, __VA_ARGS__)
#define ULOG_WARN(tag, ...)  ulog(ULOG_STDOUT | ULOG_FS, ULOG_WARN_LVL, tag, __VA_ARGS__)
#define ULOG_ERR(tag, ...)   ulog(ULOG_STDOUT | ULOG_FS, ULOG_ERR_LVL, tag, __VA_ARGS__)

/** Binary dump macros */
#define ULOG_DUMP_TRACE(tag, desc, data, len) ulog_dump(ULOG_STDOUT | ULOG_FS, ULOG_TRACE_LVL, tag, desc, data, len)
#define ULOG_DUMP_DEBUG(tag, desc, data, len) ulog_dump(ULOG_STDOUT | ULOG_FS, ULOG_DEBUG_LVL, tag, desc, data, len)
#define ULOG_DUMP_INFO(tag, desc, data, len)  ulog_dump(ULOG_STDOUT | ULOG_FS, ULOG_INFO_LVL, tag, desc, data, len)
#define ULOG_DUMP_WARN(tag, desc, data, len)  ulog_dump(ULOG_STDOUT | ULOG_FS, ULOG_WARN_LVL, tag, desc, data, len)
#define ULOG_DUMP_ERR(tag, desc, data, len)   ulog_dump(ULOG_STDOUT | ULOG_FS, ULOG_ERR_LVL, tag, desc, data, len)

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */
