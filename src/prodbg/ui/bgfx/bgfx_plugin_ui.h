#pragma once

#include "../plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BgfxPluginUI : public PluginUI
{
    void create(int width, int height);
    void destroy();

    void init(ViewPluginInstance* instance);
    State updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);

    void preUpdate();   // update before plugin update
    void postUpdate(); // update after plugins

    float getStatusBarSize();
    void setStatusTextNoFormat(const char* text);

private:
    float m_statusSize = 18.0f;
    char m_statusText[4096];
};

