#pragma once

struct ViewPluginInstance;
struct PDReader;
struct PDWriter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_init(ViewPluginInstance* instance);
void PluginUI_updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);
