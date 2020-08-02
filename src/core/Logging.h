#pragma once

#include "api/include/pd_log.h"

#if defined(__GCC__) || defined(__CLANG__)
#define LOG_PRINTF_FORMAT_ATTRIBUTE __attribute__((__format__ (__printf__, 4, 5)));
#else
#define LOG_PRINTF_FORMAT_ATTRIBUTE
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

enum class LogLevel : unsigned char { Trace, Debug, Info, Warn, Error, Fatal };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void log_func(LogLevel level, const char* file, int line, const char* format, ...) LOG_PRINTF_FORMAT_ATTRIBUTE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define trace(fmt, ...) log_func(prodbg::LogLevel::Trace, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define debug(fmt, ...) log_func(prodbg::LogLevel::Debug, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define info(fmt, ...)  log_func(prodbg::LogLevel::Info,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define warn(fmt, ...)  log_func(prodbg::LogLevel::Warn,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define error(fmt, ...) log_func(prodbg::LogLevel::Error, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define fatal(fmt, ...) log_func(prodbg::LogLevel::Fatal, __FILE__, __LINE__, fmt, __VA_ARGS__);

}


