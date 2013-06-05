#pragma once

enum  
{
    R_DEBUG, 
    R_INFO, 
    R_ERROR 
};

void log(int logLevel, const char* format, ...) __attribute__((format(printf, 2, 3)));
void log_set_level(int logLevel);
void log_level_push();
void log_level_pop();

#define log_error(...) log(R_ERROR,  __VA_ARGS__);

