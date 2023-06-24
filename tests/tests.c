
#include "ulog.h"

int main()
{
    int i = 0;

    ulog_init(ULOG_STDOUT);

    ulog(ULOG_STDOUT, ULOG_NONE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_STDOUT, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_STDOUT, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "err %d", i++);

    ulog_set_level(ULOG_INFO_LVL);

    ulog(ULOG_STDOUT, ULOG_NONE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_STDOUT, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_STDOUT, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "err %d", i++);

    ulog_set_level(ULOG_TRACE_LVL);

    ulog(ULOG_STDOUT, ULOG_NONE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_STDOUT, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_STDOUT, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_STDOUT, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "err %d", i++);

    ulog(ULOG_STDOUT, ULOG_ERR_LVL, "tag5", "very looooooooooooooooooooooooooooong striiiiiiiiiiiiiiiiiiiiiiiiiing", i++);

    ulog_init(ULOG_STDOUT | ULOG_FS);
    ulog(ULOG_FS, ULOG_NONE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_FS, ULOG_TRACE_LVL, "tag1", "trace %d", i++);
    ulog(ULOG_FS, ULOG_DEBUG_LVL, "tag2", "debug %d", i++);
    ulog(ULOG_FS, ULOG_INFO_LVL, "tag3", "info %d", i++);
    ulog(ULOG_FS, ULOG_WARN_LVL, "tag4", "warn %d", i++);
    ulog(ULOG_FS, ULOG_ERR_LVL, "tag5", "err %d", i++);
}
