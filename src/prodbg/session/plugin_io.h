#pragma once

struct PDLoadState;
struct PDSaveState;

void PluginIO_initLoadJson(PDLoadState* loadFuncs);
void PluginIO_initSaveJson(PDSaveState* saveFuncs);
