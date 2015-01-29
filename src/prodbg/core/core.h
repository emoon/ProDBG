#pragma once

#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

#if defined(__clang__) || defined(__gcc__)
#define PD_NO_RETURN __attribute__((noreturn))
#else
#define PD_NO_RETURN 
#endif


