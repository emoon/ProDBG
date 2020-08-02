#pragma once

#if defined(__GCC__) || defined(__CLANG__)
#define PD_PRINTF_FORMAT_ATTRIBUTE __attribute__((__format__ (__printf__, 5, 6)));
#else
#define PD_PRINTF_FORMAT_ATTRIBUTE
#endif

enum { PDLog_Trace, PDLog_Debug, PDLog_Info, PDLog_Warn, PDLog_Error, PDLog_Fatal };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDLog {
    /// Private data
    void* priv;
    /// Trace level logging
    void (*log_func)(void* priv, int level, const char* file, int line, const char* format, ...) PD_PRINTF_FORMAT_ATTRIBUTE;
} PDLog;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global logging functions. It requires that the plugin has a global "g_pd_log" variable to make logging easier

extern PDLog* g_pd_log;

#define pd_trace(fmt, ...) g_pd_log->log_func(g_pd_log->priv, PDLog_Trace, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_debug(fmt, ...) g_pd_log->log_func(g_pd_log->priv, PDLog_Debug, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_info(fmt, ...)  g_pd_log->log_func(g_pd_log->priv, PDLog_Info,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_warn(fmt, ...)  g_pd_log->log_func(g_pd_log->priv, PDLog_Warn,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_error(fmt, ...) g_pd_log->log_func(g_pd_log->priv, PDLog_Error, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_fatal(fmt, ...) g_pd_log->log_func(g_pd_log->priv, PDLog_Fatal, __FILE__, __LINE__, fmt, __VA_ARGS__);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Log which takes log variable instead of using the global version above

#define pd_log_trace(log, fmt, ...) log->log_func(log->priv, PDLog_Trace, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_log_debug(log, fmt, ...) log->log_func(log->priv, PDLog_Debug, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_log_info(log, fmt, ...)  log->log_func(log->priv, PDLog_Info,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_log_warn(log, fmt, ...)  log->log_func(log->priv, PDLog_Warn,  __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_log_error(log, fmt, ...) log->log_func(log->priv, PDLog_Error, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define pd_log_fatal(log, fmt, ...) log->log_func(log->priv, PDLog_Fatal, __FILE__, __LINE__, fmt, __VA_ARGS__);

