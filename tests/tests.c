
#include "ulog.h"
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

/* ---- Capture buffer for ULOG_FUNC tests ---- */
#define CAPTURE_BUF_SIZE 4096
static char capture_buf[CAPTURE_BUF_SIZE];
static size_t capture_pos = 0;

static void capture_reset(void)
{
    capture_buf[0] = '\0';
    capture_pos = 0;
}

static int capture_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(capture_buf + capture_pos,
                      CAPTURE_BUF_SIZE - capture_pos, fmt, args);
    va_end(args);
    if (n > 0)
        capture_pos += (size_t)n;
    return n;
}

/* ---- Helper ---- */
static int contains(const char* haystack, const char* needle)
{
    return strstr(haystack, needle) != NULL;
}

/* ---- Tests ---- */

static void test_func_basic(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_FUNC);
    ulog_set_level(ULOG_TRACE_LVL);

    capture_reset();
    ulog(ULOG_FUNC, ULOG_INFO_LVL, "test", "hello %d", 42);
    assert(contains(capture_buf, "hello 42"));
    printf("  PASS: test_func_basic\n");
}

static void test_func_level_filter(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_FUNC);
    ulog_set_level(ULOG_WARN_LVL);

    capture_reset();
    ulog(ULOG_FUNC, ULOG_DEBUG_LVL, "test", "should not appear");
    assert(capture_pos == 0);

    ulog(ULOG_FUNC, ULOG_ERR_LVL, "test", "should appear");
    assert(capture_pos > 0);
    assert(contains(capture_buf, "should appear"));
    printf("  PASS: test_func_level_filter\n");
}

static void test_func_not_called_when_dest_disabled(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_STDOUT);  /* only stdout, not ULOG_FUNC */
    ulog_set_level(ULOG_TRACE_LVL);

    capture_reset();
    ulog(ULOG_FUNC, ULOG_ERR_LVL, "test", "invisible");
    assert(capture_pos == 0);
    printf("  PASS: test_func_not_called_when_dest_disabled\n");
}

static void test_func_null_pointer(void)
{
    ulog_deinit();
    ulog_set_func(NULL);
    ulog_init(ULOG_FUNC);
    ulog_set_level(ULOG_TRACE_LVL);

    /* Should not crash */
    ulog(ULOG_FUNC, ULOG_ERR_LVL, "test", "no crash");
    printf("  PASS: test_func_null_pointer\n");
}

static void test_func_macros(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_FUNC);
    ulog_set_level(ULOG_TRACE_LVL);

    capture_reset();
    ULOG_TRACE("macro", "trace msg");
    assert(contains(capture_buf, "trace msg"));

    capture_reset();
    ULOG_ERR("macro", "err msg %s", "detail");
    assert(contains(capture_buf, "err msg detail"));
    printf("  PASS: test_func_macros\n");
}

static void test_func_dump(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_FUNC);
    ulog_set_level(ULOG_TRACE_LVL);

    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};

    capture_reset();
    ulog_dump(ULOG_FUNC, ULOG_INFO_LVL, "dump", "Test data", data, sizeof(data));
    assert(contains(capture_buf, "Test data"));
    assert(contains(capture_buf, "4 bytes"));
    assert(contains(capture_buf, "de ad be ef"));
    printf("  PASS: test_func_dump\n");
}

static void test_func_dump_macro(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_FUNC);
    ulog_set_level(ULOG_TRACE_LVL);

    uint8_t data[] = {0xCA, 0xFE};

    capture_reset();
    ULOG_DUMP_DEBUG("dmac", "Cafe", data, sizeof(data));
    assert(contains(capture_buf, "Cafe"));
    assert(contains(capture_buf, "ca fe"));
    printf("  PASS: test_func_dump_macro\n");
}

static void test_func_combined_dest(void)
{
    ulog_deinit();
    ulog_set_func(capture_printf);
    ulog_init(ULOG_STDOUT | ULOG_FUNC);
    ulog_set_level(ULOG_TRACE_LVL);

    capture_reset();
    ulog(ULOG_STDOUT | ULOG_FUNC, ULOG_INFO_LVL, "both", "dual output");
    assert(contains(capture_buf, "dual output"));
    printf("  PASS: test_func_combined_dest\n");
}

/* ---- Original demo tests ---- */

static void test_demo(void)
{
    int i = 0;

    ulog_deinit();
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
    printf("  PASS: test_demo\n");
}

/* ---- Main ---- */

int main(void)
{
    printf("Running ULOG_FUNC tests:\n");
    test_func_basic();
    test_func_level_filter();
    test_func_not_called_when_dest_disabled();
    test_func_null_pointer();
    test_func_macros();
    test_func_dump();
    test_func_dump_macro();
    test_func_combined_dest();

    printf("\nRunning demo tests:\n");
    test_demo();

    printf("\nAll tests passed.\n");
    return 0;
}
