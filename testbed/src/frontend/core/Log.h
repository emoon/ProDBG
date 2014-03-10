#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__clang__)
void log(int logLevel, const char* filename, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
#else
void log(int logLevel, const char* filename, int line, const char* format, ...);
#endif

void log_set_level(int logLevel);
void log_level_push();
void log_level_pop();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define log_error(...) log(LOG_ERROR,  __FILE__, __LINE__, __VA_ARGS__);
#define log_debug(...) log(LOG_DEBUG,  __FILE__, __LINE__, __VA_ARGS__);
#define log_info(...)  log(LOG_INFO,   __FILE__, __LINE__, __VA_ARGS__);

}

