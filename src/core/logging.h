#pragma once

#include "api/include/pd_log.h"

#if defined(__GCC__) || defined(__CLANG__)
#define LOG_PRINTF_FORMAT_ATTRIBUTE __attribute__((__format__ (__printf__, 4, 5)));
#else
#define LOG_PRINTF_FORMAT_ATTRIBUTE
#endif

enum class LogLevel : unsigned char { Trace, Debug, Info, Warn, Error, Fatal };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void log_func(LogLevel level, const char* file, int line, const char* format, ...) LOG_PRINTF_FORMAT_ATTRIBUTE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define log_trace(fmt, ...) log_func(LogLevel::Trace, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_debug(fmt, ...) log_func(LogLevel::Debug, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_info(fmt, ...)  log_func(LogLevel::Info,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_warn(fmt, ...)  log_func(LogLevel::Warn,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_error(fmt, ...) log_func(LogLevel::Error, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_fatal(fmt, ...) log_func(LogLevel::Fatal, __FILE__, __LINE__, fmt, __VA_ARGS__);

