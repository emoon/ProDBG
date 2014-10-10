#define STB_DEFINE
#include "stb.h"

// hack
#if defined(__linux__)
double sqrt(double v) { return 0.0; }
#endif
