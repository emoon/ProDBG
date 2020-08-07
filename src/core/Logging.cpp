#include "Logging.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

namespace prodbg {

static const char* level_strings[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
static const char* level_colors[] = { "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m" };

static int log_color = 1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void log_func(LogLevel level, const char* file, int line, const char* format, ...) {
    char buf[16];

    time_t t = time(NULL);
    auto time = localtime(&t);

    buf[strftime(buf, sizeof(buf), "%H:%M:%S", time)] = '\0';
    if (log_color) {
        printf("%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf, level_colors[(int)level], level_strings[(int)level], file, line);
    } else {
        printf("%s %-5s %s:%d: ", buf, level_strings[(int)level], file, line);
    }

    va_list arglist;
    va_start(arglist, format);
    vprintf(format, arglist);
    va_end(arglist);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDLog s_log = {
    nullptr,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDLog* log_get_api() {
    return &s_log;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
