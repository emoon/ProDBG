#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

void StatusBar_init();

#if defined(__clang__)
void StatusBar_setText(int slot, const char* format, ...) __attribute__((format(printf, 2, 3)));
#else
void StatusBar_setText(int slot, const char* format, ...);
#endif

}
