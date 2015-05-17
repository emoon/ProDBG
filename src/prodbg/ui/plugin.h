#pragma once

#include "api/include/pd_ui.h"
#include "core/math.h"

struct PDReader;
struct PDWriter;
struct PDViewPlugin;
struct PDBackendPlugin;
struct PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance
{
    PDUI ui;
    PDViewPlugin* plugin;
    FloatRect rect;
    void* userData;
    int count;
    bool markDeleted;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PluginUI
{
	enum State
	{
		None,
		CloseView,
	};

	ViewPluginInstance* createViewPlugin(PluginData* pluginData);

	virtual void init(ViewPluginInstance* instance) = 0;
	virtual State updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern PluginUI* g_pluginUI;

