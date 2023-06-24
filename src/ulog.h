/**
  ******************************************************************************
  * @file    ulog.h
  * @author  Alexander Smirnov
  * @brief   Micro logging system
  * @copyright Copyright (C) 2016, 2018, 2020, 2023 Alexander Smirnov
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
    ULOG_TRACE_LVL   = 0x01,    /**< Log trace */
    ULOG_DEBUG_LVL   = 0x02,    /**< Log debug */
    ULOG_INFO_LVL    = 0x03,    /**< Log information */
    ULOG_WARN_LVL    = 0x04,    /**< Log warning */
    ULOG_ERR_LVL     = 0x05,    /**< Log error */
};

/** Initalization */
void ulog_init(char dest);
/** Deinitalization */
void ulog_deinit(void);
/** Get current destination */
char ulog_get_dest(void);
/** Set maximum level */
void ulog_set_level(char level);
/** Get level */
char ulog_get_level(void);

/**
  * Log a message
  * @brief  System log Ex: 2000/01/01 06:03:22.000 [  protect] Overcurrent
  */
void ulog(char dest, char level, const char* tag, const char* msg, ...);

#ifdef __cplusplus
}
#endif

#endif /* _ULOG_H_ */