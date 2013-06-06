#pragma once

enum  
{
    R_DEBUG, 
    R_INFO, 
    R_ERROR 
};

void log(int logLevel, const char* filename, int line, const char* format, ...) __attribute__((format(printf, 4, 5)));
void log_set_level(int logLevel);
void log_level_push();
void log_level_pop();

#define log_error(...) log(R_ERROR,  __FILE__, __LINE__, __VA_ARGS__);

