
#include "ulog.h"
#include <time.h>

int main(void)
{
    int i = 0;

    ulog_init(ULOG_STDOUT);

    ulog(ULOG_STDOUT, ULOG_NONE_LVL, "tag0", "example none message %d", i++);
    ulog(ULOG_STDOUT, ULOG_TRACE_LVL, "tag1", "example trace message %d", i++);
    ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "tag2", "example debug message %d", i++);
    ulog(ULOG_STDOUT, ULOG_INFO_LVL, "tag3", "example info message %d", i++);
    ulog(ULOG_STDOUT, ULOG_WARN_LVL, "tag4", "example warn message %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "example err message %d", i++);

    ulog_set_level(ULOG_INFO_LVL);

    ulog(ULOG_STDOUT, ULOG_NONE_LVL, "tag0", "none %d", i++);
    ulog(ULOG_STDOUT, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_STDOUT, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_STDOUT, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "err %d", i++);

    ulog_set_level(ULOG_TRACE_LVL);

    struct timespec tim = {.tv_sec = 1, .tv_nsec = 500000};

    nanosleep(&tim, NULL);

    ulog(ULOG_STDOUT, ULOG_NONE_LVL, "tag0", "none %d", i++);
    ulog(ULOG_STDOUT, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_STDOUT, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_STDOUT, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "err %d", i++);

    ulog_set_level(ULOG_TRACE_LVL);

    ULOG_TRACE("tag1", "ULOG_TRACE %d", i++);
    ULOG_DEBUG("tag2", "ULOG_DEBUG %d", i++);
    ULOG_INFO("tag3", "ULOG_INFO %d", i++);
    ULOG_WARN("tag4", "ULOG_WARN %d", i++);
    ULOG_ERR("tag5", "ULOG_ERR %d", i++);

    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "very looooooooooooooooooooooooooooong striiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiing %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "many\nlines\nwith\ttabs %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag6", "new data %d", i++);

    ulog_deinit();
    ulog_init(ULOG_STDOUT | ULOG_FS);
    ulog(ULOG_FS, ULOG_NONE_LVL, "tag0", "none %d", i++);
    ulog(ULOG_FS, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_FS, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_FS, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_FS, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_FS, ULOG_ERR_LVL, "tag5", "err %d", i++);

    /* Binary dump examples */
    uint8_t test_data[] = {
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f,
        0x72, 0x6c, 0x64, 0x21, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
    };

    ULOG_INFO("binary", "");
    ULOG_DUMP_INFO("binary", "Test buffer", test_data, sizeof(test_data));

    ULOG_INFO("binary", "");
    uint8_t small_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
    ULOG_DUMP_DEBUG("binary", "Small buffer", small_data, sizeof(small_data));

    ULOG_INFO("binary", "");
    ulog_dump(ULOG_STDOUT, ULOG_WARN_LVL, "binary", "Warning buffer", test_data, 18);
}
