
#include "ulog.h"
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#define BENCH_ITERS  50000
#define BENCH_ROUNDS 5

/* ---- ULOG_FUNC sinks ---- */
static unsigned long sink_calls = 0;
static unsigned long sink_bytes = 0;

/* Discards the output: what is left is the cost of ulog itself */
static int sink_null(const char* fmt, ...)
{
    (void)fmt;
    sink_calls++;
    return 0;
}

/* Formats into a buffer like a real transport does */
static int sink_format(const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    sink_calls++;
    if (n > 0)
        sink_bytes += (unsigned long)n;
    return n;
}

/* ---- Runner: best round out of BENCH_ROUNDS ---- */
#define BENCH(name, call)                                                               \
    do                                                                                  \
    {                                                                                   \
        double best = 0;                                                                \
        long stdout_bytes = 0;                                                          \
        for (int r = 0; r < BENCH_ROUNDS; r++)                                          \
        {                                                                               \
            struct timespec t0, t1;                                                     \
            sink_calls = 0;                                                             \
            sink_bytes = 0;                                                             \
            rewind(stdout);                                                             \
            clock_gettime(CLOCK_MONOTONIC, &t0);                                        \
            for (int i = 0; i < BENCH_ITERS; i++)                                       \
                call;                                                                   \
            clock_gettime(CLOCK_MONOTONIC, &t1);                                        \
            stdout_bytes = ftell(stdout);                                               \
            double ns = ((double)(t1.tv_sec - t0.tv_sec) * 1e9 +                        \
                         (double)(t1.tv_nsec - t0.tv_nsec)) / BENCH_ITERS;              \
            if (r == 0 || ns < best)                                                    \
                best = ns;                                                              \
        }                                                                               \
        fprintf(out, "%-28s %10.1f %12.1f %10.2f %10.1f\n", name, best,                 \
                (double)stdout_bytes / BENCH_ITERS, (double)sink_calls / BENCH_ITERS,   \
                (double)sink_bytes / BENCH_ITERS);                                      \
    } while (0)

/* ---- Main ---- */

int main(void)
{
    uint8_t data[64];
    for (size_t i = 0; i < sizeof(data); i++)
        data[i] = (uint8_t)i;

    /* Results go to the real stdout, the log output itself to a temporary file */
    FILE* out = fdopen(dup(STDOUT_FILENO), "w");
    FILE* tmp = tmpfile();
    if (out == NULL || tmp == NULL || dup2(fileno(tmp), STDOUT_FILENO) < 0)
        return 1;

    fprintf(out, "%-28s %10s %12s %10s %10s\n", "scenario", "ns/call", "stdout bytes", "func calls", "func bytes");

    ulog_set_func(sink_null);
    ulog_init(ULOG_FUNC);

    ulog_set_level(ULOG_WARN_LVL);
    BENCH("filtered by level", ulog(ULOG_FUNC, ULOG_DEBUG_LVL, "bench", "cell %d: %d mV", i, 3300));

    ulog_set_level(ULOG_TRACE_LVL);
    BENCH("filtered by dest", ulog(ULOG_STDOUT, ULOG_INFO_LVL, "bench", "cell %d: %d mV", i, 3300));

    BENCH("func null, no args", ulog(ULOG_FUNC, ULOG_INFO_LVL, "bench", "balancing started"));
    BENCH("func null, 2 ints", ulog(ULOG_FUNC, ULOG_INFO_LVL, "bench", "cell %d: %d mV", i, 3300));

    ulog_set_func(sink_format);
    BENCH("func format, no args", ulog(ULOG_FUNC, ULOG_INFO_LVL, "bench", "balancing started"));
    BENCH("func format, 2 ints", ulog(ULOG_FUNC, ULOG_INFO_LVL, "bench", "cell %d: %d mV", i, 3300));
    BENCH("func format, float + str",
          ulog(ULOG_FUNC, ULOG_WARN_LVL, "bench", "%s: U=%.3f V I=%.2f A", "pack", 51.234, -12.5));
    BENCH("func format, dump 64 bytes",
          ulog_dump(ULOG_FUNC, ULOG_INFO_LVL, "bench", "frame", data, sizeof(data)));

    ulog_deinit();
    ulog_init(ULOG_STDOUT);
    BENCH("stdout, 2 ints", ulog(ULOG_STDOUT, ULOG_INFO_LVL, "bench", "cell %d: %d mV", i, 3300));
    BENCH("stdout, dump 64 bytes",
          ulog_dump(ULOG_STDOUT, ULOG_INFO_LVL, "bench", "frame", data, sizeof(data)));

    ulog_deinit();
    ulog_init(ULOG_STDOUT | ULOG_FUNC);
    BENCH("stdout + func, 2 ints",
          ulog(ULOG_STDOUT | ULOG_FUNC, ULOG_INFO_LVL, "bench", "cell %d: %d mV", i, 3300));

    return 0;
}
