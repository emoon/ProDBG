#pragma once

#include "../plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BgfxPluginUI : public PluginUI
{
	void init(ViewPluginInstance* instance);
	State updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);
};

