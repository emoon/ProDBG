#pragma once

struct ViewPluginInstance;
struct PDReader;
struct PDWriter;
struct FloatRect;

enum PluginUIState
{
    PluginUIState_None,
    PluginUIState_CloseView,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_init(ViewPluginInstance* instance);
void PluginUI_getWindowRect(ViewPluginInstance* instance, FloatRect* rect);
void PluginUI_setWindowRect(ViewPluginInstance* instance, FloatRect* rect);

PluginUIState PluginUI_updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);
//bool PluginUI_isActiveWindow(ViewPluginInstance* instance);

