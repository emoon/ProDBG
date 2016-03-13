#pragma once

#include <stddef.h>

void* File_loadToMemory(const char* filename, size_t* size, size_t padAllocSize);
