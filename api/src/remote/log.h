#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__clang__) || defined(__gcc__)
void pda_log_out(int logLevel, const char* filename, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
#else
void pda_log_out(int logLevel, const char* filename, int line, const char* format, ...);
#endif

void pda_log_set_level(int logLevel);
void pda_log_level_push();
void pda_log_level_pop();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define log_error(...) pda_log_out(LOG_ERROR,  __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) pda_log_out(LOG_DEBUG,  __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  pda_log_out(LOG_INFO,   __FILE__, __LINE__, __VA_ARGS__)

