# ulog
Micro log library for embedded systems.

## Using

Simple example:

```c
#include "ulog.h"
...
ulog_init(ULOG_STDOUT);

static const char* TAG = "i2c";

ULOG_INFO(TAG, "init");
ULOG_ERR(TAG, "sending rc:%d", rc);

```

Set level of logging:

```c
#include "ulog.h"
...
/* Init ulog. Print to stdout */
ulog_init(ULOG_STDOUT);
/* Set log level. Print message >= ULOG_INFO_LVL */
ulog_set_level(ULOG_INFO_LVL);

/* Will not print */
ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "spi", "send %d bytes", n);
/* Will print */
ulog(ULOG_STDOUT, ULOG_INFO_LVL, "uart", "init");
/* Will print */
ulog(ULOG_STDOUT, ULOG_WARN_LVL, "i2c", "wrong data: %s", err);
```

Log into file system or stdout:

```c
/* Init ulog. Print to stdout and file system */
ulog_init(ULOG_STDOUT | ULOG_FS);
/* Print to file system */
ulog(ULOG_STDOUT, ULOG_ERR_LVL, "protect", "overvoltage: %f", v);
/* Print to stdout and file system */
ulog(ULOG_STDOUT | ULOG_FS, ULOG_INFO_LVL, "can", "received id: %d cmd: %s", id, cmd);
```

## Examples of output

With timpestamp, level and tag:

```bash
2023/06/25 08:51:59.000 [      tag1] DEBUG example debug message 1
2023/06/25 08:51:59.000 [      tag2]  INFO example info message 2
2023/06/25 08:51:59.000 [      tag3]  WARN example warn message 3
2023/06/25 08:51:59.000 [      tag4]   ERR example err message 4
```

Without timestamp, level and tag:

```bash
(0000001344) example debug message 2
(0000001369) example info message 3
(0000001371) example warn message 4
(0000001373) example err message 5
```
