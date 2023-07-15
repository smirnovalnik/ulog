/**
  ******************************************************************************
  * @file    ulog_conf.h
  * @author  Alexander Smirnov
  * @brief   Configuration template
  * @copyright Copyright (C) 2016, 2018, 2020, 2023 Alexander Smirnov
  ******************************************************************************
*/

#ifndef _ULOG_CONF_H_
#define _ULOG_CONF_H_

// General settings

#define ULOG_PRINT_TAG          1
#define ULOG_PRINT_COLOR        1
#define ULOG_PRINT_LEVEL        1
#define ULOG_ENDLINE            "\n"
#define ULOG_REMOVE_SPACES      1
#define ULOG_TIMESTAMP_MS       1

#define ULOG_FILE_SYSTEM        1
#define ULOG_FILE_NAME          "system.log"
#define ULOG_MAX_FILE_SIZE      (10 * 1024 * 1024)   /* bytes */

// Target specific settings

#define ULOG_USE_MUTEX          1
#define ULOG_USE_POSIX_MUTEX    1
#define ULOG_USE_POSIX_IO       1

#if ULOG_USE_MUTEX == 1
#if ULOG_USE_POSIX_MUTEX == 1
#include <pthread.h>
#include <stdlib.h>
static pthread_mutex_t ulog_mutex = PTHREAD_MUTEX_INITIALIZER;
#define ULOG_CREATE_MUTEX()
#define ULOG_MUTEX_TAKE()   pthread_mutex_lock(&ulog_mutex);
#define ULOG_MUTEX_GIVE()   pthread_mutex_unlock(&ulog_mutex);
#else
#include "FreeRTOS.h"
#include "semphr.h"
static xSemaphoreHandle ulog_mutex;
#define ULOG_CREATE_MUTEX()  if (ulog_mutex == NULL) \
                                ulog_mutex = xSemaphoreCreateMutex()
#define ULOG_MUTEX_TAKE()    xSemaphoreTake(ulog_mutex, portMAX_DELAY)
#define ULOG_MUTEX_GIVE()    xSemaphoreGive(ulog_mutex)
#endif
#else
#define ULOG_CREATE_MUTEX()
#define ULOG_MUTEX_TAKE()
#define ULOG_MUTEX_GIVE()
#endif

#if ULOG_USE_POSIX_IO == 1

#else
#include "fatfs.h"      /* FatFs from Chan */
#endif

#endif /* _ULOG_CONF_H_ */
