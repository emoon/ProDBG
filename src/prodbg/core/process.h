#pragma once

typedef void* ProcessHandle;

ProcessHandle Process_spawn(const char* exe, const char** args);

int Process_wait(ProcessHandle handle);
int Process_kill(ProcessHandle handle);
