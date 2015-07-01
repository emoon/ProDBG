#pragma once

#include <stdint.h>

void Settings_registerService();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Settings_loadSettings(const char* filename); // Also support to override existing settings
void Settings_destroy();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access to settings

int64_t Settings_getInt(const char* category, const char* value);
double Settings_getReal(const char* category, const char* value);
const char* Settings_getString(const char* category, const char* value);
uint32_t Settings_getShortcut(const char* pluginId, const char* operation);
uint32_t Settings_decodeKeyCombo(const char* keyCombo);


