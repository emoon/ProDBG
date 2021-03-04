#pragma once

char* shared_object_error();
char* shared_object_exe_directory();
void* shared_object_open(const char* base_path, const char* name);
void* shared_object_symbol(void* handle, const char* symbol_name);

