#pragma once

typedef void* SharedObjectHandle;

char* shared_object_error();
char* shared_object_exe_directory();

SharedObjectHandle shared_object_open(const char* base_path, const char* name);
SharedObjectHandle shared_object_open_fullpath(const char* path);

void shared_object_close(SharedObjectHandle handle);
void* shared_object_symbol(SharedObjectHandle handle, const char* symbol_name);

