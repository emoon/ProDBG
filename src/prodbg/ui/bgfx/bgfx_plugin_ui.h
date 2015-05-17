#pragma once

#include "../plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BgfxPluginUI : public PluginUI
{
	void init(ViewPluginInstance* instance);
	State updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);

	void update();
	float getStatusBarSize();
	void setStatusTextNoFormat(const char* text);

private:
	float m_statusSize = 18.0f;
	char m_statusText[4096];
};

