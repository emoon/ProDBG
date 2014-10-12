#pragma once

struct ViewPluginInstance;
struct PDReader;
struct PDWriter;

enum PluginUIState
{
	PluginUIState_None,
	PluginUIState_CloseView,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_init(ViewPluginInstance* instance);
PluginUIState PluginUI_updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);
