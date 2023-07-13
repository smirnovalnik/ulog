
#include "ulog.h"
#include <time.h>

int main()
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

    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "very looooooooooooooooooooooooooooong striiiiiiiiiiiiiiiiiiiiiiiiiing", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "many\nlines\nwith\ttabs", i++);

    ulog_init(ULOG_STDOUT | ULOG_FS);
    ulog(ULOG_FS, ULOG_NONE_LVL, "tag0", "none %d", i++);
    ulog(ULOG_FS, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_FS, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_FS, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_FS, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_FS, ULOG_ERR_LVL, "tag5", "err %d", i++);
}
