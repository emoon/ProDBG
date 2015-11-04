#include "time.h"

#ifndef _WIN32
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Time_sleepMs(int ms) {
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000));
#endif
}


