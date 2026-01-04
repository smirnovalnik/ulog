# ulog

Micro logging library for embedded systems written in C99.

## Features

- Multiple log levels: TRACE, DEBUG, INFO, WARN, ERR
- Multiple output destinations: stdout, file system, or both
- Configurable output format: timestamp, tag, color, level
- Thread-safe logging with mutex support (POSIX or FreeRTOS)
- Binary data dump in hexadecimal format with ASCII view
- Automatic log file rotation when max size is reached
- Zero overhead when disabled (compile-time switch)
- No dynamic memory allocation
- Portable: POSIX and embedded systems (FatFs)

## Quick Start

### Basic Usage

```c
#include "ulog.h"

static const char* TAG = "i2c";

int main(void) {
    ulog_init(ULOG_STDOUT);

    ULOG_INFO(TAG, "init");
    ULOG_ERR(TAG, "sending rc:%d", rc);

    return 0;
}
```

### Log Level Filtering

```c
#include "ulog.h"

/* Init ulog. Print to stdout */
ulog_init(ULOG_STDOUT);

/* Set log level. Print message >= ULOG_INFO_LVL */
ulog_set_level(ULOG_INFO_LVL);

/* Will not print */
ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "spi", "send %d bytes", n);

/* Will print */
ulog(ULOG_STDOUT, ULOG_INFO_LVL, "uart", "init");
ulog(ULOG_STDOUT, ULOG_WARN_LVL, "i2c", "wrong data: %s", err);
```

### Multiple Destinations

```c
/* Init ulog. Print to stdout and file system */
ulog_init(ULOG_STDOUT | ULOG_FS);

/* Print to file system only */
ulog(ULOG_FS, ULOG_ERR_LVL, "protect", "overvoltage: %f", v);

/* Print to stdout and file system */
ulog(ULOG_STDOUT | ULOG_FS, ULOG_INFO_LVL, "can", "received id: %d cmd: %s", id, cmd);
```

### Binary Data Dump

```c
uint8_t buffer[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};

/* Using convenience macro */
ULOG_DUMP_INFO("spi", "RX buffer", buffer, sizeof(buffer));

/* Using function directly */
ulog_dump(ULOG_STDOUT, ULOG_DEBUG_LVL, "uart", "TX data", buffer, 32);
```

## Configuration

Copy `templates/ulog_conf.h` to your project and customize the settings:

### Main Settings

```c
#define ULOG_ENABLE             1     // Master switch (0 = disable completely)
#define ULOG_TIMESTAMP          1     // 1 = real timestamp, 0 = clock ticks
#define ULOG_TIMESTAMP_MS       1     // Add milliseconds (requires ULOG_TIMESTAMP=1)
#define ULOG_PRINT_TAG          1     // Print tag
#define ULOG_PRINT_COLOR        1     // ANSI color codes
#define ULOG_PRINT_LEVEL        1     // Print level
#define ULOG_LEVEL_SHORT        0     // Short names (T/D/I/W/E vs TRACE/DEBUG/INFO/WARN/ERR)
#define ULOG_MESSAGE_LEN        80    // Maximum message length
```

### Binary Dump Settings

```c
#define ULOG_DUMP_BYTES_PER_LINE 16   // Bytes per line (8 or 16 recommended)
#define ULOG_DUMP_SHOW_ASCII     1    // Show ASCII representation
```

### File System Settings

```c
#define ULOG_FILE_SYSTEM        0     // Enable file logging (0 for most MCUs)
#define ULOG_FILE_NAME          "system.log"
#define ULOG_MAX_FILE_SIZE      (1 * 1024 * 1024)  // 1 MB before rotation
```

### Platform Settings

```c
#define ULOG_USE_MUTEX          1     // Thread safety
#define ULOG_USE_POSIX_MUTEX    1     // 1 = POSIX pthread, 0 = FreeRTOS
#define ULOG_USE_POSIX_IO       1     // 1 = POSIX fopen/fprintf, 0 = FatFs
```

## Building

### Using Make

```bash
make        # Build and run tests
make clean  # Clean build directory
```

### Manual Compilation

```bash
gcc -std=c99 -Wall -Isrc -Itemplates src/ulog.c your_app.c -lpthread -o app
```

### Integration

Add to your project:
1. Copy `src/ulog.c` and `src/ulog.h`
2. Copy `templates/ulog_conf.h` and customize for your platform
3. Add `-Isrc -Itemplates` to include paths
4. Link with `-lpthread` (POSIX systems)

## API Reference

### Initialization

```c
void ulog_init(uint8_t dest);           // Initialize with destination(s)
void ulog_deinit(void);                 // Deinitialize
uint8_t ulog_get_dest(void);            // Get current destination
```

### Level Control

```c
void ulog_set_level(uint8_t level);     // Set minimum log level
uint8_t ulog_get_level(void);           // Get current level
```

### Logging

```c
void ulog(uint8_t dest, uint8_t level, const char* tag, const char* msg, ...);
void ulog_dump(uint8_t dest, uint8_t level, const char* tag, const char* desc, const void* data, size_t len);
```

### Convenience Macros

```c
ULOG_TRACE(tag, format, ...)    // Trace level
ULOG_DEBUG(tag, format, ...)    // Debug level
ULOG_INFO(tag, format, ...)     // Info level
ULOG_WARN(tag, format, ...)     // Warning level
ULOG_ERR(tag, format, ...)      // Error level

ULOG_DUMP_TRACE(tag, desc, data, len)
ULOG_DUMP_DEBUG(tag, desc, data, len)
ULOG_DUMP_INFO(tag, desc, data, len)
ULOG_DUMP_WARN(tag, desc, data, len)
ULOG_DUMP_ERR(tag, desc, data, len)
```

## Output Examples

### Text Logging

With timestamp, level, tag, and color:

```
2023/06/25 08:51:59.000 [      tag1] TRACE example trace message 1
2023/06/25 08:51:59.000 [      tag2] DEBUG example debug message 2
2023/06/25 08:51:59.000 [      tag3]  INFO example info message 3
2023/06/25 08:51:59.000 [      tag4]  WARN example warn message 4
2023/06/25 08:51:59.000 [      tag5]   ERR example err message 5
```

With clock ticks only:

```
(0000001344) example debug message 2
(0000001369) example info message 3
(0000001371) example warn message 4
(0000001373) example err message 5
```

### Binary Dump

```
2026/01/04 14:30:15.123 [    binary]  INFO Test buffer (48 bytes)
  0000: 48 65 6c 6c 6f 20 57 6f 72 6c 64 21 00 01 02 03  Hello World!....
  0010: 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13  ................
  0020: 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10  ................
```

## License

Copyright (C) 2016 - 2026 Alexander Smirnov. See LICENSE file for details.
