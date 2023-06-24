/**
  ******************************************************************************
  * @file    ulog_conf.h
  * @author  Alexander Smirnov
  * @brief   Micro logging system
  * @copyright Copyright (C) 2016, 2018, 2020, 2023 Alexander Smirnov
  ******************************************************************************
*/

#define ULOG_PRINT_TAG          1
#define ULOG_PRINT_COLOR        1
#define ULOG_PRINT_LEVEL        1
#define ULOG_ENDLINE            "\n"
#define ULOG_REMOVE_SPACES      0
#define ULOG_USE_MUTEX          0

#define ULOG_FILE_SYSTEM        1
#define ULOG_FILE_NAME          "system.log"
#define ULOG_MAX_FILE_SIZE      (10 * 1024 * 1024)   /* bytes */
#define ULOG_USE_POSIX          1

#if ULOG_USE_MUTEX == 1
#include "FreeRTOS.h"
#include "semphr.h"
static xSemaphoreHandle ulog_mutex;
#define ULOG_CREATE_MUTEX()  if (ulog_mutex == NULL)\
                                ulog_mutex = xSemaphoreCreateMutex()
#define ULOG_MUTEX_TAKE()    xSemaphoreTake(ulog_mutex, portMAX_DELAY)
#define ULOG_MUTEX_GIVE()    xSemaphoreGive(ulog_mutex)
#else
#define ULOG_CREATE_MUTEX()
#define ULOG_MUTEX_TAKE()
#define ULOG_MUTEX_GIVE()
#endif

#if ULOG_USE_POSIX == 1

#else
#include "fatfs.h"      /* FatFs from Chan */
#endif