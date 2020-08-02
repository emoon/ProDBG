#pragma once

#if defined(__GCC__) || defined(__CLANG__)
#define PD_PRINTF_FORMAT_ATTRIBUTE __attribute__((__format__ (__printf__, 2, 3)));
#else
#define PD_PRINTF_FORMAT_ATTRIBUTE
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDLog {
    /// Private data
    void* priv;
    /// Trace level logging
    void (*log_info)(void* priv, const char* format, ...) PD_PRINTF_FORMAT_ATTRIBUTE;
    /// Warning level logging
    void (*log_warn)(void* priv, const char* format, ...) PD_PRINTF_FORMAT_ATTRIBUTE;
    /// Info level logging
    void (*log_trace)(void* priv, const char* format, ...) PD_PRINTF_FORMAT_ATTRIBUTE;
    /// Error level logging
    void (*log_error)(void* priv, const char* format, ...) PD_PRINTF_FORMAT_ATTRIBUTE;
} PDLog;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global logging functions. It requires that the plugin has a global "g_pd_log" variable to make logging easier

extern PDLog* g_pd_log;

#define pd_info(fmt, ...) g_pd_log->log_info(g_pd_log->priv, fmt, __VA_ARGS__);
#define pd_warn(fmt, ...) g_pd_log->log_warn(g_pd_log->priv, fmt, __VA_ARGS__);
#define pd_trace(fmt, ...) g_pd_log->log_trace(g_pd_log->priv, fmt, __VA_ARGS__);
#define pd_error(fmt, ...) g_pd_log->log_error(g_pd_log->priv, fmt, __VA_ARGS__);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Log which takes log variable instead of using the global version above

#define pd_log_info(log, fmt, ...) log->log_info(log->priv, fmt, __VA_ARGS__);
#define pd_log_warn(log, fmt, ...) log->log_warn(log->priv, fmt, __VA_ARGS__);
#define pd_log_trace(log, fmt, ...) log->log_trace(log->priv, fmt, __VA_ARGS__);
#define pd_log_error(log, fmt, ...) log->log_error(log->priv, fmt, __VA_ARGS__);

